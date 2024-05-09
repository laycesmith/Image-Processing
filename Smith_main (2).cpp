/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Layce Smith

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    I think I got pretty much everything. Struggled a bit with the error notifications, but I anticipated as much as I could.

- Did you do any optional enhancements? If so, please explain:
    Sorry, no. My brain can't handle anymore. :)
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <string>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//

//Process 1
vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
        
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            double row_middle;
            double column_middle;
            
            if (num_rows % 2 != 0)
            {
                row_middle = num_rows / 2 + .5;
            }
            else if (num_rows % 2 == 0)
            {
                row_middle = num_rows / 2;
            }
            
            if (num_columns % 2 != 0)
            {
                column_middle = num_columns / 2 + .5;
            }
            else if (num_columns % 2 == 0)
            {
                column_middle = num_columns / 2;
            }
            
    
            double row_difference = row - row_middle;
            double column_difference = col - column_middle;
            
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            
            double distance = sqrt(pow(column_difference, 2) + pow(row_difference, 2));
            double scaling_factor = (num_rows - distance) / num_rows;
            
            new_image[row][col].blue = blue_color * scaling_factor;
            new_image[row][col].green = green_color * scaling_factor;
            new_image[row][col].red = red_color * scaling_factor;

        }
        cout << endl;
    }
    return new_image;
}

// Process 2

vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
        
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            //get current pixel
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            
            //get the average value
            
            double average = (blue_color + green_color + red_color) / 3;
            
            //if cell is light, make it lighter
            if (average >= 170)
            {
                new_image[row][col].blue = (255 - (255 - blue_color) * scaling_factor);
                new_image[row][col].green = (255 - (255 - green_color) * scaling_factor);
                new_image[row][col].red = (255 - (255 - red_color) * scaling_factor);
            }
            else if (average < 90)
            {
                new_image[row][col].blue = blue_color * scaling_factor;
                new_image[row][col].green = green_color * scaling_factor;
                new_image[row][col].red = red_color * scaling_factor;
            }
            else
            {
                new_image[row][col].blue = blue_color;
                new_image[row][col].green = green_color;
                new_image[row][col].red = red_color;
            }
        }
        cout << endl;
    }
    return new_image;
}

// Process 3

vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
        
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            //get current pixel
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            
            //get the average value
            double gray_value = (blue_color + green_color + red_color) / 3;
            
            //set new colors to gray_value
            new_image[row][col].blue = gray_value;
            new_image[row][col].green = gray_value;
            new_image[row][col].red = gray_value;
        }
        cout << endl;
    }
    return new_image;
}

// Process 4

vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();
    
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));
    
    
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {                     
           new_image[col][row] = image[height - 1 - row][col];
        }
    }
    return new_image;
}

//Process 5

vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number)
{
    int height = image.size();
    int width = image[0].size();
    
    int angle = number * 90;
    
    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));
    
    if (angle % 90 != 0)
    {
        cout << "angle must be a multiple of 90 degrees." << endl;
    }
    else if (angle % 360 == 0)
    {
        return image;
    }
    else if (angle % 360 == 90)
    {
        return process_4(image);
    }
    else if (angle % 360 == 180)
    {
        return process_4(process_4(image));
    }
    else
    {
        return process_4(process_4(process_4(image)));
    }
    
    return image;
}

// Process 6

vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale)
{
    int height = image.size();
    int width = image[0].size();
    
    vector<vector<Pixel>> new_image(height * y_scale, vector<Pixel>(width * x_scale));
    
     for (int row = 0; row < height * y_scale; row++)
    {
        for (int col = 0; col < width * x_scale; col++)
        {                     
           new_image[row][col] = image[row / y_scale][col / x_scale];
        }
    }
    return new_image;
}

// Process 7
                     
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image) 
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            //get current pixel
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            
            //get the average value
            
            double average = (blue_color + green_color + red_color) / 3;
            
             if (average >= 255 / 2)
            {
                new_image[row][col].blue = 255;
                new_image[row][col].green = 255;
                new_image[row][col].red = 255;
            }
            else
            {
                new_image[row][col].blue = 0;
                new_image[row][col].green = 0;
                new_image[row][col].red = 0;
            }
        }
        cout << endl;
    }
    return new_image;
}

// Process 8

vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor) 
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            
            new_image[row][col].blue = (255 - (255 - blue_color) * scaling_factor);
            new_image[row][col].green = (255 - (255 - green_color) * scaling_factor);
            new_image[row][col].red = (255 - (255 - red_color) * scaling_factor);
        }
        cout << endl;
    }
    return new_image;
}

// Process 9

vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
        
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {   
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;

            new_image[row][col].blue = blue_color * scaling_factor;
            new_image[row][col].green = green_color * scaling_factor;
            new_image[row][col].red = red_color * scaling_factor;
        }
        cout << endl;
    }
    return new_image;
}

// Process 10

vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            //get current pixel
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            
            int max_color = blue_color;
            
            if (green_color > max_color)
            {
                max_color = green_color;
            }
            else
            {
                max_color = red_color;
            }
            
            if (blue_color + green_color + red_color >= 550)
            {
                new_image[row][col].blue = 255;
                new_image[row][col].green = 255;
                new_image[row][col].red = 255;
            }
            else if (blue_color + green_color + red_color <= 150)
            {
                new_image[row][col].blue = 0;
                new_image[row][col].green = 0;
                new_image[row][col].red = 0;
            }
            else if (max_color == red_color)
            {
                new_image[row][col].blue = 255;
                new_image[row][col].green = 0;
                new_image[row][col].red = 0;
            }
            else if (max_color == green_color)
            {
                new_image[row][col].blue = 0;
                new_image[row][col].green = 255;
                new_image[row][col].red = 0;
            }
            else
            {
                new_image[row][col].blue = 0;
                new_image[row][col].green = 0;
                new_image[row][col].red = 255;
            }
        }
        cout << endl;
    }
    return new_image;
}

int main()
{
    
    string input = "";
    
    int value;
    string filename;
    do
    {
        cout << "Enter a BMP file name of an image that you would like to edit." << endl;
        cin >> filename;
        if (cin.fail() || filename.substr(filename.length() - 4) != ".bmp")
        {
            cin.clear();
            string item;
            cin >> item;
            cout << "Error: invalid file name.";
            value = -1;
        }
        else
        {
            value = 0;
         }
    }
    while (value < 0);
    
    vector<vector<Pixel>> image = read_image(filename);
    
    int iterator = 0;
            
    while (iterator == 0 && input != "Q")
    {
        cout << "Image Processing Menu" << endl;
        cout << "0) Change image" << endl;
        cout << "1) Vignette" << endl;
        cout << "2) Clarendon" << endl;
        cout << "3) Grayscale" << endl;
        cout << "4) Rotate 90 degrees" << endl;
        cout << "5) Rotate multiple 90 degrees" << endl;
        cout << "6) Enlarge" << endl;
        cout << "7) High contrast" << endl;
        cout << "8) Lighten" << endl;
        cout << "9) Darken" << endl;
        cout << "10) Black, white, red, green, blue" << endl;
    
        cout << endl;
        cout << "Enter your selection (Q to quit):" << endl;
        cin >> input;
        cout << endl;
        
        
        int repeat = 1;
        while (repeat > 0)
        {
            if (input != "0" && input != "1" && input != "2" && input != "3" && input != "4" && input != "5" && input != "6" && input != "7" && input != "8" && input != "9" && input != "10" && input != "Q")
            {
                cout << "Oops! Please select an option between 0 and 10 (Q to quit)." << endl;
                cin >> input;
            }
            else
            {
                repeat--;
            }
        }
        iterator = 0;
                
        
        if (input == "0")
        {
            int value;
            string filename;
            do
            {
                cout << "Enter a BMP file name of an image that you would like to edit." << endl;
                cin >> filename;
                if (cin.fail() || filename.substr(filename.length() - 4) != ".bmp")
                {
                    cin.clear();
                    string item;
                    cin >> item;
                    cout << "Error: invalid file name.";
                    value = -1;
                }
                else
                {
                    value = 0;
                }
            }
            while (value < 0);
    
            vector<vector<Pixel>> image = read_image(filename);
            int iterator = 0;
        }
        
        if (input == "1")
        {
            vector<vector<Pixel>> new_image = process_1(image);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully applied vignette!" << endl;
        }
        
        if (input == "2")
        {
            int value;
            double scaling_factor;
            do
            {
                cout << "Select a scaling factor between 0.1 and 0.9." << endl;
                cin >> scaling_factor;
                if (cin.fail())
                {
                    cin.clear();
                    string item;
                    cin >> item;
                    cout << "Error: invalid input.";
                    value = -1;
                }
                else if (scaling_factor > 0 || scaling_factor < 0.1 || scaling_factor > 0.9)
                {
                    cout << "Error: invalid input.";
                    value = -1;
                }
                else
                {
                    value = 0;
                }
            }
            while (value < 0);
                
            vector<vector<Pixel>> new_image = process_2(image, scaling_factor);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully applied clarendon!" << endl;
        }
        
        if (input == "3")
        {
            vector<vector<Pixel>> new_image = process_3(image);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully applied grayscale!" << endl;
        }
        
        if (input == "4")
        {
            vector<vector<Pixel>> new_image = process_4(image);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully applied 90 degree rotation!" << endl;
        }
        
        if (input == "5")
        {
            cout << "How many 90 degree rotations?" << endl;
            string number;
            cin >> number;
            
            int repeat = 1;
            while (repeat > 0)
            {
                for (int i = 0; i < number.length(); i++)
                {
                    if (number[i] != '0' && number[i] != '1' && number[i] != '2' && number[i] != '3' && number[i] != '4' && number[i] != '5' && number[i] != '6' && number[i] != '7' && number[i] != '8' && number[i] != '9')
                    {
                        cout << "Error: invalid input. Please enter a number." << endl;
                        cin >> number;
                    }
                    else
                    {
                        repeat--;
                    }
                }   
            }
            int x = atoi(number.c_str());
            vector<vector<Pixel>> new_image = process_5(image, x);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully applied multiple 90 degree rotation!" << endl;
        }    
        
        if (input == "6")
        {
            cout << "How would you like to enlarge the image?" << endl;
            string x;
            cin >> x;
            int repeat = 1;
            while (repeat > 0)
            {
                for (int i = 0; i < x.length(); i++)
                {
                    if (x[i] != '0' && x[i] != '1' && x[i] != '2' && x[i] != '3' && x[i] != '4' && x[i] != '5' && x[i] != '6' && x[i] != '7' && x[i] != '8' && x[i] != '9')
                    {
                        cout << "Error: invalid input. Please enter a number." << endl;
                        cin >> x;
                    }
                    else
                    {
                        repeat--;
                    }
                }   
            }
            int new_x = atoi(x.c_str());
            
            
            string y;
            cin >> y;
            int cont = 1;
            while (cont > 0)
            {
                for (int i = 0; i < y.length(); i++)
                {
                    if (y[i] != '0' && y[i] != '1' && y[i] != '2' && y[i] != '3' && y[i] != '4' && y[i] != '5' && y[i] != '6' && y[i] != '7' && y[i] != '8' && y[i] != '9')
                    {
                        cout << "Error: invalid input. Please enter a number." << endl;
                        cin >> y;
                    }
                    else
                    {
                        cont--;
                    }
                }   
            }
            int new_y = atoi(y.c_str());
            
            vector<vector<Pixel>> new_image = process_6(image, new_x, new_y);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully enlarged!" << endl;
        }
        
        if (input == "7")
        {
            vector<vector<Pixel>> new_image = process_7(image);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully applied high contrast!" << endl;
        }
        
        if (input == "8")
        {
            vector<vector<Pixel>> new_image = process_8(image, 0.5);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully lightened!" << endl;
        }
        
        if (input == "9")
        {
            vector<vector<Pixel>> new_image = process_9(image, 0.5);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully darkened!" << endl;
        }
        
        if (input == "10")
        {
            vector<vector<Pixel>> new_image = process_10(image);
            bool success = write_image("new_file.bmp", new_image);
            cout << "Successfully applied black, white, red, green, blue filter!" << endl;
        }
    }
    
    cout << "Thank you for using my program! Quitting..." << endl;
    return true;
}
    
