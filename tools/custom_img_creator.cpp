#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <fstream>  // Include the necessary header for file operations
#include "lodepng/lodepng.h"

// Function to find the index of the closest color in the palette
size_t findClosestColor(const std::vector<unsigned char>& color, const std::vector<std::vector<unsigned char>>& palette) {
    size_t closestIndex = 0;
    double minDistance = std::numeric_limits<double>::max();

    for (size_t i = 0; i < palette.size(); ++i) {
        double distance = 0.0;

        for (size_t j = 0; j < color.size(); ++j) {
            distance += std::pow(color[j] - palette[i][j], 2);
        }

        distance = std::sqrt(distance);

        if (distance < minDistance) {
            minDistance = distance;
            closestIndex = i;
        }
    }

    return closestIndex;
}

// Function to perform k-means clustering on the palette
std::vector<std::vector<unsigned char>> kMeansClustering(const std::vector<std::vector<unsigned char>>& palette, size_t k, size_t maxIterations) {
    std::vector<std::vector<unsigned char>> centroids(k);
    
    // Initialize centroids randomly
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, palette.size() - 1);

    for (size_t i = 0; i < k; ++i) {
        centroids[i] = palette[dist(gen)];
    }

    for (size_t iteration = 0; iteration < maxIterations; ++iteration) {
        // Assign each color to the closest centroid
        std::vector<std::vector<unsigned char>> clusters(k, std::vector<unsigned char>(4, 0));

        for (const auto& color : palette) {
            size_t closestIndex = findClosestColor(color, centroids);
            clusters[closestIndex] = color;
        }

        // Update centroids based on the mean of assigned colors
        for (size_t i = 0; i < k; ++i) {
            for (size_t j = 0; j < clusters[i].size(); ++j) {
                centroids[i][j] = clusters[i][j];
            }
        }
    }

    return centroids;
}

// Bilinear interpolation for resizing the image
unsigned char bilinearInterpolation(double x, double y, const std::vector<unsigned char>& image, unsigned width, unsigned height, unsigned channel) {
    unsigned x1 = static_cast<unsigned>(std::floor(x));
    unsigned y1 = static_cast<unsigned>(std::floor(y));
    unsigned x2 = std::min(x1 + 1, width - 1);
    unsigned y2 = std::min(y1 + 1, height - 1);

    double weight1 = (x2 - x) * (y2 - y);
    double weight2 = (x - x1) * (y2 - y);
    double weight3 = (x2 - x) * (y - y1);
    double weight4 = (x - x1) * (y - y1);

    double interpolatedValue = weight1 * image[(y1 * width + x1) * 4 + channel] +
                               weight2 * image[(y1 * width + x2) * 4 + channel] +
                               weight3 * image[(y2 * width + x1) * 4 + channel] +
                               weight4 * image[(y2 * width + x2) * 4 + channel];

    return static_cast<unsigned char>(interpolatedValue);
}

// Function to resize the image using bilinear interpolation
std::vector<unsigned char> resizeImage(const std::vector<unsigned char>& originalImage, unsigned originalWidth, unsigned originalHeight, unsigned targetWidth, unsigned targetHeight) {
    std::vector<unsigned char> resizedImage(targetWidth * targetHeight * 4);

    for (unsigned y = 0; y < targetHeight; ++y) {
        for (unsigned x = 0; x < targetWidth; ++x) {
            double xOriginal = static_cast<double>(x) / targetWidth * originalWidth;
            double yOriginal = static_cast<double>(y) / targetHeight * originalHeight;

            resizedImage[(y * targetWidth + x) * 4] = bilinearInterpolation(xOriginal, yOriginal, originalImage, originalWidth, originalHeight, 0);
            resizedImage[(y * targetWidth + x) * 4 + 1] = bilinearInterpolation(xOriginal, yOriginal, originalImage, originalWidth, originalHeight, 1);
            resizedImage[(y * targetWidth + x) * 4 + 2] = bilinearInterpolation(xOriginal, yOriginal, originalImage, originalWidth, originalHeight, 2);
            resizedImage[(y * targetWidth + x) * 4 + 3] = bilinearInterpolation(xOriginal, yOriginal, originalImage, originalWidth, originalHeight, 3);
        }
    }

    return resizedImage;
}

// Function to save the image in the custom format
void saveCustomFormat(const char* outputFilename, const std::vector<unsigned char>& resizedImage, const std::vector<std::vector<unsigned char>>& palette, unsigned targetWidth, unsigned targetHeight) {
    std::ofstream outputFile(outputFilename);

    // Write dimensions to the file
    outputFile << targetWidth << "x" << targetHeight << "\n";

    // Write palette entries to the file
    for (const auto& color : palette) {
        outputFile << static_cast<int>(color[0]) << ", " << static_cast<int>(color[1]) << ", " << static_cast<int>(color[2]) << ", " << static_cast<int>(color[3]) << "\n";
    }

    // Add an empty line
    outputFile << "\n";

    // Write image data to the file
    for (size_t y = 0; y < targetHeight; ++y) {
        for (size_t x = 0; x < targetWidth; ++x) {
            size_t index = (y * targetWidth + x) * 4;
            size_t closestIndex = findClosestColor({resizedImage[index], resizedImage[index + 1], resizedImage[index + 2], resizedImage[index + 3]}, palette);
            outputFile << closestIndex << " ";
        }
        outputFile << "\n";
    }

    std::cout << "Image saved in custom format: " << outputFilename << std::endl;
}

// Function to load PNG file, resize, create a reduced palette using k-means clustering, and save the new PNG file
void resizeAndReduceColors(const char* inputFilename, const char* outputFilename, size_t targetWidth, size_t targetHeight, size_t k, size_t maxIterations) {
    // Vector to store RGBA values for each pixel in the image
    std::vector<unsigned char> originalImage;
    unsigned originalWidth, originalHeight;

    // Load PNG file using lodepng
    unsigned error = lodepng::decode(originalImage, originalWidth, originalHeight, inputFilename);

    // Check for errors
    if (error) {
        std::cerr << "Error loading PNG file: " << lodepng_error_text(error) << std::endl;
        return;
    }

    // Check if the image has an alpha channel (RGBA)
    if (originalImage.size() % 4 != 0) {
        std::cerr << "The image does not have an alpha channel (RGBA)" << std::endl;
        return;
    }

    // Resize the image
    std::vector<unsigned char> resizedImage = resizeImage(originalImage, originalWidth, originalHeight, targetWidth, targetHeight);

    // Create a palette from the resized image
    std::vector<std::vector<unsigned char>> palette;

    // Iterate through the pixels and add unique colors to the palette
    for (size_t i = 0; i < resizedImage.size(); i += 4) {
        std::vector<unsigned char> color = {resizedImage[i], resizedImage[i + 1], resizedImage[i + 2], resizedImage[i + 3]};
        if (std::find(palette.begin(), palette.end(), color) == palette.end()) {
            palette.push_back(color);
        }
    }

    // If the palette has more than k colors, reduce it using k-means clustering
    if (palette.size() > k) {
        palette = kMeansClustering(palette, k, maxIterations);
    }

    // Save the new image in custom format
    saveCustomFormat(outputFilename, resizedImage, palette, targetWidth, targetHeight);
}

int main(int argc, char* argv[]) {
    // Check if the correct number of command-line arguments are provided
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " input_file output_file width height colors reduction" << std::endl;
        return EXIT_FAILURE;
    }

    // Extract values from command-line arguments
    const char* inputFileName = argv[1];
    const char* outputFileName = argv[2];
    int width = std::stoi(argv[3]);
    int height = std::stoi(argv[4]);
    int colors = std::stoi(argv[5]);
    int reduction = std::stoi(argv[6]);

    // Call the resizeAndReduceColors function
    resizeAndReduceColors(inputFileName, outputFileName, width, height, colors, reduction);

    return 0;
}
