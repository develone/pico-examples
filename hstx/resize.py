from PIL import Image

# 1. Open the original image
image = Image.open('lena_rgb_2048.bmp')

# 2. Resize to specific dimensions (width, height)
resized_image = image.resize((640, 480))

# 3. Save the new image
resized_image.save('resized_image.bmp')

