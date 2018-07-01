#define VIOLET_IMPLEMENTATION
#define VIOLET_NO_GUI
#include "violet.h"

#include "testfit/types.h"
#include "testfit/measure.h"
#include "testfit/utility.h"
#include "testfit/opts.h"
#include "testfit/mass_placement.h"
#include "testfit/mp_utility.h"
#include "testfit/debug.h"
#include "testfit/poly.h"

int tfmp_generate_bldg(const v2f *v, u32 n, r32 mass_width, r32 mass_height,
                       r32 aspect_ratio)
{
	temp_memory_mark_t tmem_mark;
	array(v2f) boundary;
	opts_t opts;
	mp_basis_t basis;
	array(mp_graph_t) graphs;
	array(v2f*) extrusions;
	int ret = 1;

	for (u32 i = 0; i < n; ++i)
		printf("<%.0f,%.0f>,", v[i].x, v[i].y);
	printf("\n");
	goto outx;

	vlt_init(VLT_THREAD_MAIN);
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
	opts.min_unit_width = 10.f;
	opts.add_balconies = false;

	basis.boundary = boundary;
	basis.garage = NULL;
	basis.opts = &opts;

	array_init(graphs, 4);

	try {
		mp_generate_spines(basis, &graphs);
		if (array_empty(graphs))
			goto out;

		array_init(extrusions, 2);

		mp_extrude_depr(graphs[0].guides, opts.corridor_width/2,
		                opts.path_width/2, &extrusions);
		ret = 0;
	} catch {
		goto out;
	} finally;

out:
	temp_memory_restore(tmem_mark);
	vlt_destroy(VLT_THREAD_MAIN);
outx:
	return ret;
}
