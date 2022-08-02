#include "image.h"


Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width*height];
	memset(pixels, 0, width * height * sizeof(Color));
}

//copy constructor
Image::Image(const Image& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
}

//assign operator
Image& Image::operator = (const Image& c)
{
	if(pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height*sizeof(Color)];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
	return *this;
}

Image::~Image()
{
	if(pixels) 
		delete pixels;
}



//change image size (the old one will remain in the top-left corner)
void Image::resize(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for(unsigned int x = 0; x < min_width; ++x)
		for(unsigned int y = 0; y < min_height; ++y)
			new_pixels[ y * width + x ] = getPixel(x,y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

//change image size and scale the content
void Image::scale(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];

	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			new_pixels[ y * width + x ] = getPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)) );

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::getArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
{
	Image result(width, height);
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++x)
		{
			if( (x + start_x) < this->width && (y + start_y) < this->height) 
				result.setPixel( x, y, getPixel(x + start_x,y + start_y) );
		}
	return result;
}

void Image::flipX()
{
	for(unsigned int x = 0; x < width * 0.5; ++x)
		for(unsigned int y = 0; y < height; ++y)
		{
			Color temp = getPixel(width - x - 1, y);
			setPixel( width - x - 1, y, getPixel(x,y));
			setPixel( x, y, temp );
		}
}

void Image::flipY()
{
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height * 0.5; ++y)
		{
			Color temp = getPixel(x, height - y - 1);
			setPixel( x, height - y - 1, getPixel(x,y) );
			setPixel( x, y, temp );
		}
}


//Loads an image from a TGA file
bool Image::loadTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int bytesPerPixel;
	unsigned int imageSize;

	FILE * file = fopen(filename, "rb");
   	if ( file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
		fread(header, 1, sizeof(header), file) != sizeof(header))
	{
		std::cerr << "File not found: " << filename << std::endl;
		if (file == NULL)
			return NULL;
		else
		{
			fclose(file);
			return NULL;
		}
	}

	TGAInfo* tgainfo = new TGAInfo;
    
	tgainfo->width = header[1] * 256 + header[0];
	tgainfo->height = header[3] * 256 + header[2];
    
	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (header[4] != 24 && header[4] != 32))
	{
		std::cerr << "TGA file seems to have errors or it is compressed, only uncompressed TGAs supported" << std::endl;
		fclose(file);
		delete tgainfo;
		return NULL;
	}
    
	tgainfo->bpp = header[4];
	bytesPerPixel = tgainfo->bpp / 8;
	imageSize = tgainfo->width * tgainfo->height * bytesPerPixel;
    
	tgainfo->data = new unsigned char[imageSize];
    
	if (tgainfo->data == NULL || fread(tgainfo->data, 1, imageSize, file) != imageSize)
	{
		if (tgainfo->data != NULL)
			delete tgainfo->data;
            
		fclose(file);
		delete tgainfo;
		return false;
	}

	fclose(file);

	//save info in image
	if(pixels)
		delete pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width*height];

	//convert to float all pixels
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			this->setPixel(x , height - y - 1, Color( tgainfo->data[pos+2], tgainfo->data[pos+1], tgainfo->data[pos]) );
		}

	delete tgainfo->data;
	delete tgainfo;

	return true;
}

// Saves the image to a TGA file
bool Image::saveTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	FILE *file = fopen(filename, "wb");
	if ( file == NULL )
	{
		fclose(file);
		return false;
	}

	unsigned short header_short[3];
	header_short[0] = width;
	header_short[1] = height;
	unsigned char* header = (unsigned char*)header_short;
	header[4] = 24;
	header[5] = 0;

	//tgainfo->width = header[1] * 256 + header[0];
	//tgainfo->height = header[3] * 256 + header[2];

	fwrite(TGAheader, 1, sizeof(TGAheader), file);
	fwrite(header, 1, 6, file);

	//convert pixels to unsigned char
	unsigned char* bytes = new unsigned char[width*height*3];
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[(height-y-1)*width+x];
			unsigned int pos = (y*width+x)*3;
			bytes[pos+2] = c.r;
			bytes[pos+1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width*height*3, file);
	fclose(file);
	return true;
}

void Image::bresenhamTriangle(int x0, int y0, int x1, int y1, Color c, std::vector<sCelda>& table)
// Aquesta funció ens serveix per a mappejar valors a una taula. 
{
	int dx, dy, inc_E, inc_NE, d, x, y;
	int i;
	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	if (dy > dx) {
		inc_E = 2 * dx;
		inc_NE = 2 * (dx - dy);
		d = 2 * dx - dy;
	}
	else {
		inc_E = 2 * dy;
		inc_NE = 2 * (dy - dx);
		d = 2 * dy - dx;
	}
	x = x0;
	y = y0;
	if (y >= 0 && y < this->height) {
		if (x < table[y].minx) table[y].minx = x;
		if (x > table[y].maxx) table[y].maxx = x;
	}
	while (y0 > y1 ? y > y1 : y < y1) {
		if (d <= 0) { //Choose E
			d = d + inc_E;
			if (dx > dy) x = x0 > x1 ? x - 1 : x + 1;
			else y = y0 > y1 ? y - 1 : y + 1;
		}
		else { //Choose NE
			d = d + inc_NE;
			x = x0 > x1 ? x - 1 : x + 1;
			y = y0 > y1 ? y - 1 : y + 1;
		}
		if (y >= 0 && y < this->height) {
			if (x < table[y].minx) table[y].minx = x;
			if (x > table[y].maxx) table[y].maxx = x;
		}
	}
}

void Image::drawTriangle(Vector3 p1, Vector3 p2, Vector3 p3, Vector2 t1, Vector2 t2, Vector2 t3, FloatImage* zbuffer, Image* texture, Light* light, Material* material, Vector3 p_w1, Vector3 p_w2, Vector3 p_w3, Vector3 ambient_color, Camera* camera, Image* texture_normals)
//Donat tres punt i un boleà aquesta funció ens dibuixa un triangle.
{
	std::vector<sCelda> table;
	table.resize(this->height);
	//init table
	for (int i = 0; i < table.size(); i++)
	{
		table[i].minx = 100000; //very big number
		table[i].maxx = -100000; //very small number
	}
	int x1 = p1.x; int y1 = p1.y;
	int x2 = p2.x; int y2 = p2.y;
	int x3 = p3.x; int y3 = p3.y;
	///////////// Càlculs colors per interpolacio baricentrica
	Vector2 v0 = Vector2(x2, y2) - Vector2(x1, y1);
	Vector2 v1 = Vector2(x3, y3) - Vector2(x1, y1);
	float d00 = v0.dot(v0);
	float d01 = v0.dot(v1);
	float d11 = v1.dot(v1);
	float denom = d00 * d11 - d01 * d01;
	float z = 0.0;
	Color t;
	Color c1 = Color::RED;
	Color c2 = Color::BLUE;
	Color c3 = Color::GREEN;
	Color c;
	Vector3 l;
	Color n;
	//Vector3 normal;
	/////////////
	//… raster triangle edges algorithm
	this->bresenhamTriangle(x1, y1, x2, y2, c, table);
	this->bresenhamTriangle(x1, y1, x3, y3, c, table);
	this->bresenhamTriangle(x2, y2, x3, y3, c, table);
	//… fill rows using minxX and maxX
	for (int j = 0; j < this->height; j++) {
		if (table[j].maxx > -10000 && table[j].minx < 10000) {
			for (int i = table[j].minx; i <= table[j].maxx; i++) {
				Vector2 v2 = Vector2(i, j) - Vector2(x1, y1);
				float d20 = v2.dot(v0);
				float d21 = v2.dot(v1);
				//Calculem els increments per píxel.
				float v = (d11 * d20 - d01 * d21) / denom;
				float w = (d00 * d21 - d01 * d20) / denom;
				float u = (1.0 - v - w);
				//Calculem el color segons els pesos.
				c = c1 * u + c2 * v + c3 * w;
				// Calculem la z del pixel en el que estem
				z = p1.z * u + p2.z * v + p3.z * w;

				Vector3 punt_mon = Vector3(p_w1.x * u + p_w2.x * v + p_w3.x * w, p_w1.y * u + p_w2.y * v + p_w3.y * w, p_w1.z * u + p_w2.z * v + p_w3.z * w);

				// Aprofitant els pesos calculem la x i la y en la imatge de textura del pixel dins el triangle en el que estem 
				float newX = (t1.x * u + t2.x * v + t3.x * w) *texture->width;
				float newY = (t1.y * u + t2.y * v + t3.y * w) *texture->height;

				if (newX < texture->width && newX >= 0) {
					if (newY < texture->height && newY >= 0) {
						if (i < this->width && i >= 0) {
							if (zbuffer->getPixel(i, j) >= z) { // Mirem si el pixel que volem pintar esta més aprop que el que hi ha posat al zbuffer
								zbuffer->setPixel(i, j, z);		// Si la seva z és més petita (esta més aprop de la camera) actualitzem el valor del zbuffer
								t = texture->getPixel(newX, newY);
								n = texture_normals->getPixel(newX, newY);
								Vector3 normal = Vector3((n.r/255.0 * 2) - 1, (n.g/255.0 * 2) - 1, (n.b/255.0 * 2) - 1).normalize();
								l = camera->calcLight(punt_mon, normal, light, material, ambient_color);
								float red = clamp(t.r * l.x, 0, 255);
								float green = clamp(t.g * l.y, 0, 255);
								float blue = clamp(t.b * l.z, 0, 255);
								t = Color(red, green, blue);
								paint_point(i, j, t);
							}
						}
					}
				}
			}
		}
	}
}

void Image::paint_point(int x, int y, Color c) {
	//Donat un punt i un color pintem un píxel.
	if (x < this->width && x >= 0 && y < this->height && y >= 0) setPixel(x, y, c);
}


FloatImage::FloatImage(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new float[width*height];
	memset(pixels, 0, width * height * sizeof(float));
}

//copy constructor
FloatImage::FloatImage(const FloatImage& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width*height];
		memcpy(pixels, c.pixels, width*height * sizeof(float));
	}
}

//assign operator
FloatImage& FloatImage::operator = (const FloatImage& c)
{
	if (pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width*height * sizeof(float)];
		memcpy(pixels, c.pixels, width*height * sizeof(float));
	}
	return *this;
}

FloatImage::~FloatImage()
{
	if (pixels)
		delete pixels;
}


//change image size (the old one will remain in the top-left corner)
void FloatImage::resize(unsigned int width, unsigned int height)
{
	float* new_pixels = new float[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for (unsigned int x = 0; x < min_width; ++x)
		for (unsigned int y = 0; y < min_height; ++y)
			new_pixels[y * width + x] = getPixel(x, y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}


#ifndef IGNORE_LAMBDAS

//you can apply and algorithm for two images and store the result in the first one
//forEachPixel( img, img2, [](Color a, Color b) { return a + b; } );
template <typename F>
void forEachPixel(Image& img, const Image& img2, F f) {
	for(unsigned int pos = 0; pos < img.width * img.height; ++pos)
		img.pixels[pos] = f( img.pixels[pos], img2.pixels[pos] );
}

#endif