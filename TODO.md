# campello_image — TODO

## Image format support

- [x] **Basis Universal (`.basis`) support**  
  Runtime transcoding to BC, ETC2, EAC, ASTC, and RGBA8 via basis_universal transcoder.

- [x] **KTX2 (`.ktx2`) support**  
  Parse KTX2 containers with Basis Universal payloads (ETC1S and UASTC) and transcode to GPU formats.

- [x] **HDR (Radiance `.hdr`) support**  
  Decode Greg Ward's RGBE format to RGBA32F. Required for IBL / environment maps in campello_renderer.

- [x] **OpenEXR (`.exr`) support**  
  Decode OpenEXR via tinyexr v1.0.0 (vendored). Needed for high-dynamic-range environment maps and 32-bit float textures.
