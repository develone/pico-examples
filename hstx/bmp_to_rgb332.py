import sys
import os
from PIL import Image

def convert_bmp_to_rgb332(image_path):
    if not os.path.exists(image_path):
        print(f"Error: File '{image_path}' not found.")
        return

    # Open image and force it to RGB mode
    img = Image.open(image_path).convert('RGB')
    width, height = img.size
    
    # Generate the header file name
    base_name = os.path.splitext(os.path.basename(image_path))[0]
    header_path = f"{base_name}_rgb332.h"
    
    with open(header_path, "w") as f:
        # Write guard bands and metadata
        f.write(f"#ifndef {base_name.upper()}_RGB332_H\n")
        f.write(f"#define {base_name.upper()}_RGB332_H\n\n")
        f.write(f"#define {base_name.upper()}_WIDTH  {width}\n")
        f.write(f"#define {base_name.upper()}_HEIGHT {height}\n\n")
        f.write(f"const unsigned char {base_name}_data[] = {{\n")
        
        # Process every single pixel
        pixels = list(img.getdata())
        for i, (r, g, b) in enumerate(pixels):
            # Scale 8-bit color channels down to 3-3-2 bit segments
            r_3bit = (r >> 5) & 0x07  # Keep top 3 bits
            g_3bit = (g >> 5) & 0x07  # Keep top 3 bits
            b_2bit = (b >> 6) & 0x03  # Keep top 2 bits
            
            # Pack into a single 8-bit byte
            rgb332_byte = (r_3bit << 5) | (g_3bit << 2) | b_2bit
            
            # Format output beautifully
            f.write(f"0x{rgb332_byte:02X}, ")
            if (i + 1) % 16 == 0:  # New line every 16 pixels
                f.write("\n")
                
        f.write("\n};\n\n#endif\n")
    print(f"Successfully converted! Saved to: {header_path}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python bmp_to_rgb332.py <path_to_bitmap.bmp>")
    else:
        convert_bmp_to_rgb332(sys.argv[1])
