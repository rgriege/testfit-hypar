int  tfmp_generate_bldg(const v2f *v, u32 n, r32 mass_width, r32 mass_height,
                        r32 aspect_ratio, v2f **poly_verts, u32 *num_verts,
                        u32 **poly_lengths, u32 *num_lengths);
void tfmp_free(v2f *poly_verts, u32 *poly_lengths);
