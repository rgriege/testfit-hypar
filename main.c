#include "violet.h"
#include "testfit/types.h"
#include "testfit/measure.h"
#include "testfit/opts.h"
#include "testfit/opt.h"
#include "tfmp.h"
#include "json/json.h"

int tfmp__generate_bldg(const v2f *v, u32 n, r32 mass_width, r32 mass_height,
                        r32 aspect_ratio, v2f **poly_verts, u32 *num_verts,
                        u32 **poly_lengths, u32 *num_lengths);

struct params
{
	array(v2f) boundary;
	r32 width;
	r32 height;
	r32 aspect;
};

static
void print_polygon(const char *lbl, const v2f *p, u32 n)
{
	printf("%s: ", lbl);
	for (u32 i = 0; i < n; ++i)
		printf("<%.0f,%.0f>,", p[i].x, p[i].y);
	printf("\n");
}

static
v2f geo_to_m(v2d geo)
{
	const r64 R_MAJOR = 6378137.0;
	const r64 R_MINOR = 6356752.3142;
	const r64 RATIO = R_MINOR / R_MAJOR;
	const r64 ECCENT = sqrt(1.0 - (RATIO * RATIO));
	const r64 COM = 0.5 * ECCENT;
	const r64 PI_2 = PI / 2.0;

	r64 lat = clamp(-89.5, geo.y, 89.5);
	r64 phi = lat * DEG2RAD;
	r64 sinphi = sin(phi);
	r64 con = ECCENT * sinphi;
	con = pow(((1.0 - con) / (1.0 + con)), COM);
	r64 ts = tan(0.5 * (PI_2 - phi)) / con;
	v2f m;
	m.x = R_MAJOR * geo.x * DEG2RAD;
	m.y = -R_MAJOR * log(ts);
	return m;
}

static
char *file_get_contents(const char *fname, allocator_t *allocator)
{
	char *contents = NULL;
	long len;
	FILE *fp = fopen(fname, "r");
	if (!fp)
		goto out;

	if (fseek(fp, 0, SEEK_END) != 0)
		goto err_file;

	len = ftell(fp);
	if (len <= 0)
		goto err_file;

	if (fseek(fp, 0, SEEK_SET) != 0)
		goto err_file;

	contents = amalloc(len + 1, allocator);
	contents[0] = '\0';

	if (fread(contents, 1, (size_t)len, fp) != (size_t)len) {
		afree(contents, allocator);
		goto out;
	}

	contents[len] = '\0';

err_file:
	fclose(fp);
out:
	return contents;
}

static
r32 json_float(const struct json_number_s *value)
{
	r32 result;
	const char c = value->number[value->number_size];
	((char*)value->number)[value->number_size] = '\0';
	result = strtof(value->number, NULL);
	((char*)value->number)[value->number_size] = c;
	return result;
}

static
r64 json_double(const struct json_number_s *value)
{
	r64 result;
	const char c = value->number[value->number_size];
	((char*)value->number)[value->number_size] = '\0';
	result = strtod(value->number, NULL);
	((char*)value->number)[value->number_size] = c;
	return result;
}

static
b32 json_name_eq(const struct json_string_s *name, const char *str)
{
	return strncmp(name->string, str, name->string_size) == 0;
}

static
void parse_coordinate(const struct json_array_s *array, array(v2d) *coords)
{
	const struct json_array_element_s *elem;
	v2d v;

	if (array->length != 2) {
		printf("Invalid coordinate array length\n");
		return;
	}

	elem = array->start;
	if (elem->value->type != json_type_number) {
		printf("Invalid coordinate array element type\n");
		return;
	}
	v.x = json_double(elem->value->payload);

	elem = elem->next;
	if (elem->value->type != json_type_number) {
		printf("Invalid coordinate array element type\n");
		return;
	}
	v.y = json_double(elem->value->payload);

	array_append(*coords, v);
}

static
void parse_coordinates2(const struct json_array_s *array, array(v2f) *boundary)
{
	const struct json_array_element_s *elem;
	array(v2d) coords;
	v2f offset;

	array_init_ex(coords, array->length, g_temp_allocator);

	elem = array->start;
	while (elem) {
		if (elem->value->type != json_type_array) {
			printf("Invalid coordinates polygon array element type\n");
			goto out;
		}
		parse_coordinate(elem->value->payload, &coords);
		elem = elem->next;
	}

	if (array_empty(coords))
		goto out;

	offset = geo_to_m(coords[0]);
	array_foreach(coords, const v2d, coord)
		array_append(*boundary, v2f_sub(geo_to_m(*coord), offset));

	if (point_eq((*boundary)[0], array_last(*boundary)))
		array_pop(*boundary);
	else
		array_clear(*boundary);
out:
	array_destroy(coords);
}

static
void parse_coordinates(const struct json_array_s *array, array(v2f) *boundary)
{
	const struct json_array_element_s *elem;

	if (array->length != 1) {
		printf("Invalid coordinates array length\n");
		return;
	}

	elem = array->start;
	if (elem->value->type != json_type_array) {
		printf("Invalid coordinates array element type\n");
		return;
	}

	parse_coordinates2(elem->value->payload, boundary);
}

static
void parse_geometry(const struct json_object_s *object, array(v2f) *boundary)
{
	const struct json_object_element_s *elem;

	elem = object->start;
	while (elem) {
		if (   elem->value->type == json_type_array
		    && json_name_eq(elem->name, "coordinates"))
			parse_coordinates(elem->value->payload, boundary);
		elem = elem->next;
	}
}

static
void parse_boundary(const struct json_array_s *array, array(v2f) *boundary)
{
	const struct json_array_element_s *array_elem;
	const struct json_object_s *object;
	const struct json_object_element_s *object_elem;

	if (array->length != 1) {
		printf("Invalid boundary array length\n");
		return;
	}

	array_elem = array->start;
	if (array_elem->value->type != json_type_object) {
		printf("Invalid boundary array element type\n");
		return;
	}

	object = array_elem->value->payload;

	object_elem = object->start;
	while (object_elem) {
		const struct json_string_s *name = object_elem->name;
		if (   object_elem->value->type == json_type_object
		    && json_name_eq(object_elem->name, "geometry"))
			parse_geometry(object_elem->value->payload, boundary);
		object_elem = object_elem->next;
	}
}

static b32
load(const char *fname, struct params *params)
{
	char *contents;
	struct json_value_s *root_value;
	const struct json_object_s *root;
	const struct json_object_element_s *elem;

	array_clear(params->boundary);

	contents = file_get_contents(fname, g_allocator);
	if (!contents) {
		printf("file_get_contents() failed\n");
		goto err_file;
	}

	root_value = json_parse(contents, strlen(contents));
	if (!root_value) {
		printf("json_parse() failed\n");
		goto err_parse;
	}

	root = root_value->payload;
	elem = root->start;
	while (elem) {
		if (elem->value->type == json_type_number) {
		  if (json_name_eq(elem->name, "width")) {
				params->width = json_float(elem->value->payload);
				printf("width: %f\n", params->width);
		  } else if (json_name_eq(elem->name, "height")) {
				params->height = json_float(elem->value->payload);
				printf("height: %f\n", params->height);
		  } else if (json_name_eq(elem->name, "aspectRatio")) {
				params->aspect = json_float(elem->value->payload);
				printf("aspect: %f\n", params->aspect);
			} else {
				printf("unknown number %.*s\n", (int)elem->name->string_size, elem->name->string);
			}
		} else if (elem->value->type == json_type_array) {
		  if (json_name_eq(elem->name, "boundary")) {
				parse_boundary(elem->value->payload, &params->boundary);
				if (!array_empty(params->boundary))
					print_polygon("boundary", A2PN(params->boundary));
			}
		}
		elem = elem->next;
	}

	free(root_value);
err_parse:
	afree(contents, g_allocator);
err_file:
	return !array_empty(params->boundary);
}

int main(int argc, char *const argv[])
{
	const v2f boundary_static[] = {
		{ .x=0.0f,   .y=0.0f },
		{ .x=200.0f, .y=0.0f },
		{ .x=200.0f, .y=200.0f },
		{ .x=0.0f,   .y=200.0f },
	};
	struct params params = {
		.boundary = array_create(),
		.width = 60.f,
		.height = 40.f,
		.aspect = 0.5f,
	};
	v2f *poly_verts;
	u32 *poly_lengths;
	u32 num_verts, num_lengths;
	int ret;
	u32 vert_start = 0;

	vlt_init(VLT_THREAD_MAIN);
	set_measurement_system(MEASURE_METRIC);
	opt_set_measurement_system(MEASURE_METRIC);

	log_add_std(LOG_STDOUT);

	if (argc > 1) {
		printf("loading file %s\n", argv[1]);
		if (load(argv[1], &params)) {
			if (!polyf_is_cc(A2PN(params.boundary)))
				array_reverse(params.boundary);
		} else {
			printf("load(%s) failed\n", argv[1]);
			goto out;
		}
	} else {
		array_set_sz(params.boundary, countof(boundary_static));
		memcpy(params.boundary, boundary_static, sizeof(boundary_static));
	}

	ret = tfmp__generate_bldg(A2PN(params.boundary), params.width,
	                          params.height, params.aspect,
	                          &poly_verts, &num_verts,
	                          &poly_lengths, &num_lengths);

	for (u32 i = 0; i < num_lengths; ++i) {
		print_polygon("Polygon", &poly_verts[vert_start], poly_lengths[i]);
		vert_start += poly_lengths[i];
	}

out:
	array_destroy(params.boundary);
	vlt_destroy(VLT_THREAD_MAIN);
	return ret;
}
