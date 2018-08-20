#define VIOLET_IMPLEMENTATION
#define VIOLET_NO_GUI
#define VIOLET_NO_MAIN
#include "violet.h"

#include "testfit/types.h"
#include "testfit/measure.h"
#include "testfit/utility.h"
#include "testfit/opts.h"
#include "testfit/opt.h"
#include "testfit/mass_placement.h"
#include "testfit/mp_utility.h"
#include "testfit/debug.h"
#include "testfit/poly.h"
#include "tfmp.h"

static
void linearize_extrusion_buffers(array(v2f*) extrusions,
                                 v2f **poly_verts, u32 *num_verts,
                                 u32 **poly_lengths, u32 *num_lengths)
{
	v2f *pvert;
	u32 *plen;

	*num_lengths = array_sz(extrusions);
	*num_verts = 0;
	array_foreach(extrusions, v2f*, extrusion)
		*num_verts += array_sz(*extrusion);

	*poly_verts = malloc(*num_verts * sizeof(v2f));
	*poly_lengths = malloc(*num_lengths * sizeof(u32));

	pvert = *poly_verts;
	plen  = *poly_lengths;
	array_foreach(extrusions, v2f*, extrusion) {
		const u32 len = array_sz(*extrusion);
		memcpy(pvert, *extrusion, len * sizeof(v2f));
		pvert += len;
		*plen++ = len;
	}
}

int tfmp__generate_bldg(const v2f *v, u32 n, r32 mass_width, r32 mass_height,
                        r32 aspect_ratio, v2f **poly_verts, u32 *num_verts,
                        u32 **poly_lengths, u32 *num_lengths)
{
	temp_memory_mark_t tmem_mark;
	array(v2f) boundary;
	opts_t opts;
	mp_basis_t basis;
	array(mp_graph_t) graphs;
	array(v2f*) extrusions;
	int ret = 1;

	tmem_mark = temp_memory_save(g_temp_allocator);

	array_init(boundary, n);
	array_set_sz(boundary, n);
	memcpy(boundary, v, n * sizeof(*v));

	opts_init(&opts);
	opts_default(&opts);
	opts.garage_levels_below_grade = 0;
	opts.garage_levels_above_grade = 0;
	opts.corridor_width = mass_width;
	opts.wrap_levels = 1;
	opts.podium_levels = 0;
	opts.floor2floor = mass_height;
	opts.aspect = aspect_ratio;
	opts.firewalls_enabled = false;
	opts.stairs_enabled = false;
	opts.lifts_enabled = false;
	opts.num_amenities = 0;
	opts.units_enabled = false;
	opts.min_unit_width = 3.f; // MARK
	opts.add_balconies = false;

	basis.boundary = boundary;
	basis.garage = NULL;
	basis.opts = &opts;

	array_init(graphs, 4);

	array_init(extrusions, 2);

	try {
		mp_generate_spines(basis, &graphs);
		if (array_empty(graphs))
			goto out;

		mp_extrude_depr(graphs[0].guides, opts.corridor_width/2,
		                opts.path_width/2, &extrusions);

		linearize_extrusion_buffers(extrusions, poly_verts, num_verts,
		                            poly_lengths, num_lengths);
		ret = 0;
	} catch {
		goto out;
	} finally;

out:
	array_foreach(extrusions, v2f*, extrusion)
		array_destroy(*extrusion);
	array_destroy(extrusions);
	array_foreach(graphs, mp_graph_t, graph)
		mp_graph_destroy(graph);
	array_destroy(graphs);
	opts_destroy(&opts);
	array_destroy(boundary);
	temp_memory_restore(tmem_mark);
	return ret;
}

int tfmp_generate_bldg(const v2f *v, u32 n, r32 mass_width, r32 mass_height,
                       r32 aspect_ratio, v2f **poly_verts, u32 *num_verts,
                       u32 **poly_lengths, u32 *num_lengths)
{
	int ret;
	vlt_init(VLT_THREAD_MAIN);
	set_measurement_system(MEASURE_METRIC);
	opt_set_measurement_system(MEASURE_METRIC);
	ret = tfmp__generate_bldg(v, n, mass_width, mass_height, aspect_ratio,
	                          poly_verts, num_verts, poly_lengths, num_lengths);
	vlt_destroy(VLT_THREAD_MAIN);
	return ret;
}

void tfmp_free(v2f *poly_verts, u32 *poly_lengths)
{
	free(poly_verts);
	free(poly_lengths);
}
