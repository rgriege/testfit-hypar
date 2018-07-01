#include "violet.h"

int tfmp_generate_bldg(const v2f *v, u32 n, r32 mass_width, r32 mass_height,
                       r32 aspect_ratio, v2f **poly_verts, u32 *num_verts,
                       u32 **poly_lengths, u32 *num_lengths);

int main(int argc, char *const argv[])
{
	const v2f boundary[] = {
		{ .x=0.0f,   .y=0.0f },
		{ .x=200.0f, .y=0.0f },
		{ .x=200.0f, .y=200.0f },
		{ .x=0.0f,   .y=200.0f },
	};
	v2f *poly_verts;
	u32 *poly_lengths;
	u32 num_verts, num_lengths;
	int ret;
	u32 vert_start = 0;

	ret = tfmp_generate_bldg(B2PC(boundary), 60.f, 40.f, .5f,
	                         &poly_verts, &num_verts,
	                         &poly_lengths, &num_lengths);

	for (u32 i = 0; i < num_lengths; ++i) {
		printf("Polygon: ");
		for (u32 j = 0; j < poly_lengths[i]; ++j) {
			const v2f p = poly_verts[vert_start+j];
			printf("<%.0f,%.0f>,", p.x, p.y);
		}
		printf("\n");
		vert_start += poly_lengths[i];
	}

	return ret;
}
