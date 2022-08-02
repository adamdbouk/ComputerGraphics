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
		for(unsigned int y = 0; y < height; ++y)
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

void Image::DDA(int x1, int y1, int x2, int y2, Color c)
// Donat el punt inicial i el final d'una línea ens la dibuixa d'un determinat color.
{
	float d, x, y;
	float dx = (x2 - x1);
	float dy = (y2 - y1);
	//Nombre total d'iteracions que necessitem.
	if (fabs(dx) >= fabs(dy))
		d = fabs(dx);
	else
		d = fabs(dy);
	//Calculem l'increment
	float vx = dx / d;
	float vy = dy / d;
	//Calculem la posició inicial.
	x = x1 + sgn(x1) * 0.5;
	y = y1 + sgn(y1) * 0.5;
	//Pintem un píxel a cada iteració.
	for (int i = 0; i <= d; i++)
	{
		paint_point(floor(x), floor(y), c);
		x = x + vx;
		y = y + vy;
	}
}

int Image::sgn(float num) {
	//Donat un número retorna 1 si és major o igual a 0 i -1 si és negatiu.
	if (num >= 0) return 1;
	else  return -1;
}

void Image::paint_point(int x, int y, Color c) {
	//Donat un punt i un color pintem un píxel.
	if (x < this->width && x > 0 && y < this->height && y > 0) setPixel(x, y, c);
}

void Image::bresenham(int x0, int y0, int x1, int y1, Color c) {
	// Donat el punt inicial i el final d'una línea ens la dibuixa d'un determinat color.
	int dx, dy, inc_E, inc_NE, d, x, y;
	int i;
	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	inc_E = 2 * dy;
	inc_NE = 2 * (dy - dx);
	d = 2 * dy - dx; //Quants píxels hem de pintar
	x = x0;
	y = y0;
	paint_point(x, y, c);
	if (dy > dx) { //Condició pel segon, tercer, sisé i seté octats
		inc_E = 2 * dx;
		inc_NE = 2 * (dx - dy);
		d = 2 * dx - dy;
		while (y0 > y1 ? y > y1 : y < y1) 
		{
			if (d <= 0) { //Choose E
				d = d + inc_E;
				y = y0 > y1 ? y - 1 : y + 1; // Si estem al segon o al tercer octant sumem la y, sino restem.
			}
			else { //Choose NE
				d = d + inc_NE;
				x = x > x1 ? x - 1 : x + 1;  //Si estem al al tercer o al sisé octant restem la x, sino sumem.
				y = y0 > y1 ? y - 1 : y + 1; // Si estem al segon o al tercer octant sumem la y, sino restem.
			}
			paint_point(x, y, c);
		}
	}
	else { // Pels altres octants.
		while (x < x1) // Si estem al primer o al vuité
		{
			if (d <= 0) { //Choose E
				d = d + inc_E;
				x = x + 1;
			}
			else { //Choose NE
				d = d + inc_NE;
				x = x + 1;
				y = y0 > y1 ? y - 1 : y + 1; // Si estem al primer sumem, sino restem la y.
			}
			paint_point(x, y, c);
		}
		while (x > x1) // Si estem al quart o al cinqué
		{
			if (d <= 0) { //Choose E
				d = d + inc_E;
				x = x - 1;
			}
			else { //Choose NE
				d = d + inc_NE;
				x = x - 1;
				y = y = y0 > y1 ? y - 1 : y + 1; // Si estem al quart sumem la y, sino la restem.
			}
			paint_point(x, y, c);
		}
	}
}

void Image::bresenhamCircle(int cX, int cY, int r, Color c, bool fill)
//Donat un centre y un radi aquesta funció ens dibuixarà un cercle.
{
	paint_point(cX, cY, Color::CYAN);
	int x, y;
	int v;
	x = 0;
	y = r;
	v = 1 - r;
	// Dibuixem el punt i la seva reflexió a cada octant.
	if (fill) {
		for (int i = -x + cX; i <= x + cX; i++) {
			paint_point(i, y + cY, c);			  // 1r 4t
			paint_point(i, -y + cY, c);			  // 5e 8e
		}

		for (int i = -y + cX; i <= y + cX; i++) {
			paint_point(i, x + cY, c);			  // 2n 3r
			paint_point(i, -x + cY, c);			  // 6e 7e
		}
	}
	else {
		paint_point(x + cX, y + cY, c);		// 1r
		paint_point(y + cX, x + cY, c);		// 2n
		paint_point(-y + cX, x + cY, c);	// 3r
		paint_point(-x + cX, y + cY, c);	// 4t
		paint_point(-x + cX, -y + cY, c);	// 5e
		paint_point(-y + cX, -x + cY, c);	// 6e
		paint_point(y + cX, -x + cY, c);	// 7e
		paint_point(x + cX, -y + cY, c);	// 8e
	}
	while (y > x) {
		if (v < 0) {
			v = v + 2 * x + 3;
			x++;
		}
		else {
			v = v + 2 * (x - y) + 5;
			x++;
			y--;
		}
		if (fill) {
			for (int i = -x + cX; i <= x + cX; i++) { 
				paint_point(i, y + cY, c);			  // 1r 4t
				paint_point(i, -y + cY, c);			  // 5e 8e
			}

			for (int i = -y + cX; i <= y + cX; i++) {
				paint_point(i, x + cY, c);			  // 2n 3r
				paint_point(i, -x + cY, c);			  // 6e 7e
			}
		}
		else {
			paint_point(x + cX, y + cY, c);		// 1r
			paint_point(y + cX, x + cY, c);		// 2n
			paint_point(-y + cX, x + cY, c);	// 3r
			paint_point(-x + cX, y + cY, c);	// 4t
			paint_point(-x + cX, -y + cY, c);	// 5e
			paint_point(-y + cX, -x + cY, c);	// 6e
			paint_point(y + cX, -x + cY, c);	// 7e
			paint_point(x + cX, -y + cY, c);	// 8e
		}
	}
}

void Image::bresenhamTriangle(int x0, int y0, int x1, int y1, Color c, std::vector<sCelda>& table) 
// Aquesta funció ens serveix per a mappejar valors a una taula. 
{
	int dx, dy, inc_E, inc_NE, d, x, y;
	int i;
	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	inc_E = 2 * dy;
	inc_NE = 2 * (dy - dx);
	d = 2 * dy - dx;
	x = x0;
	y = y0;
	paint_point(x, y, c);
	if (y >= 0 && y < this->height) {
		if (x < table[y].minx) table[y].minx = x;
		if (x > table[y].maxx) table[y].maxx = x;
	}
	if (dy > dx) {
		inc_E = 2 * dx;
		inc_NE = 2 * (dx - dy);
		d = 2 * dx - dy;
		while (y0 > y1 ? y > y1 : y < y1)
		{
			if (d <= 0) { //Choose E
				d = d + inc_E;
				//x = x + 1;
				y = y0 > y1 ? y - 1 : y + 1;
			}
			else { //Choose NE
				d = d + inc_NE;
				x = x > x1 ? x - 1 : x + 1;;
				y = y0 > y1 ? y - 1 : y + 1;
			}
			paint_point(x, y, c);
			if (y >= 0 && y < this->height) {
				if (x < table[y].minx) table[y].minx = x;
				if (x > table[y].maxx) table[y].maxx = x;
			}
		}
	}
	else {
		while (x < x1)
		{
			if (d <= 0) { //Choose E
				d = d + inc_E;
				x = x + 1;
			}
			else { //Choose NE
				d = d + inc_NE;
				x = x + 1;
				y = y0 > y1 ? y - 1 : y + 1;
			}
			paint_point(x, y, c);
			if (y >= 0 && y < this->height) {
				if (x < table[y].minx) table[y].minx = x;
				if (x > table[y].maxx) table[y].maxx = x;
			}
		}
		while (x > x1)
		{
			if (d <= 0) { //Choose E
				d = d + inc_E;
				x = x - 1;
			}
			else { //Choose NE
				d = d + inc_NE;
				x = x - 1;
				y = y = y0 > y1 ? y - 1 : y + 1;
			}
			paint_point(x, y, c);
			if (y >= 0 && y < this->height) {
				if (x < table[y].minx) table[y].minx = x;
				if (x > table[y].maxx) table[y].maxx = x;
			}
		}
	}
}

void Image::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color c, bool fill, bool flat)
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
	///////////// Càlculs colors per interpolacio baricentrica
	Vector2 v0 = Vector2(x2, y2) - Vector2(x1, y1);
	Vector2 v1 = Vector2(x3, y3) - Vector2(x1, y1);
	float d00 = v0.dot(v0);
	float d01 = v0.dot(v1);
	float d11 = v1.dot(v1);
	float denom = d00 * d11 - d01 * d01;
	Color c1 = Color::RED;
	Color c2 = Color::BLUE;
	Color c3 = Color::GREEN;
	/////////////
	//… raster triangle edges algorithm
	this->bresenhamTriangle(x1, y1, x2, y2, c, table);
	this->bresenhamTriangle(x1, y1, x3, y3, c, table);
	this->bresenhamTriangle(x2, y2, x3, y3, c, table);
	//… fill rows using minxX and maxX
	for (int j = 0; j < this->height; j++) {
		if (table[j].maxx > -10000 && table[j].minx < 10000) {
			if (fill) {
				if (flat) {
					//Pintem el triangle d'un mateix color.
					for (int i = table[j].minx; i <= table[j].maxx; i++) paint_point(i, j, c);
				}
				else {
					for (int i = table[j].minx; i <= table[j].maxx; i++) {
						Vector2 v2 = Vector2(i, j) - Vector2(x1, y1);
						float d20 = v2.dot(v0);
						float d21 = v2.dot(v1);
						//Calculem els increments per píxel.
						float v = (d11 * d20 - d01 * d21) / denom;
						float w = (d00 * d21 - d01 * d20) / denom;
						float u = 1.0 - v - w;
						//Calculem el color segons els pesos.
						c = c1 * u + c2 * v + c3 * w;
						paint_point(i, j, c);
					}
				}
			}
			else {
				paint_point(table[j].maxx, j, c);
				paint_point(table[j].minx, j, c);
			}
		}
	}
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