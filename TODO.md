# campello_image — TODO

## Image format support

- [ ] **HDR (Radiance `.hdr`) support**  
  Decode Greg Ward's RGBE format to RGBA16F or RGBA8. Required for IBL / environment maps in campello_renderer.

- [ ] **OpenEXR (`.exr`) support**  
  Decode OpenEXR via tinyexr or similar. Needed for high-dynamic-range environment maps and 16-bit/32-bit float textures.
