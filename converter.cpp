#include <heif.h>
#include <jpeglib.h>
#include <iostream>
#include <fstream>
#include <vector>

bool heic_to_jpg(const char* input_path, const char* output_path) {
    heif_context* ctx = heif_context_alloc();
    heif_context_read_from_file(ctx, input_path, nullptr);
    
    heif_image_handle* handle;
    heif_context_get_primary_image_handle(ctx, &handle);
    
    heif_image* img;
    heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);
    
    int width = heif_image_get_width(img, heif_channel_interleaved);
    int height = heif_image_get_height(img, heif_channel_interleaved);
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, nullptr);
    
    FILE* outfile = fopen(output_path, "wb");
    if (!outfile) {
        std::cerr << "Error opening output file: " << output_path << std::endl;
        return false;
    }
    
    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);
    
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);
    
    JSAMPROW row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = const_cast<uint8_t*>(&data[cinfo.next_scanline * width * 3]);
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    
    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);
    
    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input.heic> <output.jpg>" << std::endl;
        return 1;
    }
    
    const char* input_file = argv[1];
    const char* output_file = argv[2];
    
    if (heic_to_jpg(input_file, output_file)) {
        std::cout << "Converted " << input_file << " to " << output_file << std::endl;
    } else {
        std::cerr << "Failed to convert " << input_file << std::endl;
        return 1;
    }
    
    return 0;
}
