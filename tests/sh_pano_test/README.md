Original panorama used to calculate the spherical harmonics coefficients

![original](/tests/sh_pano_test/img/_orig.png)

Image synthesis from SH coefficients (no weights)

![no weights](/tests/sh_pano_test/img/_synth.png)

Image synthesis from SH coefficients with irradiance weights
as described in 
[An Efficient Representation for Irradiance Environment Maps](https://cseweb.ucsd.edu/~ravir/papers/envmap/envmap.pdf) formulas (8), (9)
and
[A Signal-Processing Framework for Inverse Rendering](https://graphics.stanford.edu/papers/invrend/invrend.pdf) (15), (16)

![irradiance weights](/tests/sh_pano_test/img/_irr.png)

Image synthesis from SH coefficients with Phong weights
as described in 
[A Signal-Processing Framework for Inverse Rendering](https://graphics.stanford.edu/papers/invrend/invrend.pdf) formula (19)

![Phong weights](/tests/sh_pano_test/img/_phong.png)
