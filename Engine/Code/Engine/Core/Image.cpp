#include "Engine/Core/Image.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

Image::~Image()
{

}

Image::Image(char const* imageFilePath)
	:m_imageFilePath(imageFilePath)
{
	int width;
	int height;
	int numColorChannels;

	// Loads the image 
	unsigned char* imageData = stbi_load(imageFilePath, &width, &height, &numColorChannels, STBI_default);

	if (imageData)
	{
		// Set dimensions
		m_dimensions = IntVec2(width, height);

		// Reserve space in the vector
		size_t texelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
		m_rgbaTexels.reserve(texelCount);

		// Iterate through the texel data and convert to Rgba8
		if (numColorChannels == 3)
		{
			unsigned char* tempImageData = imageData;
			m_rgbaTexels.resize(texelCount);

			for (int texelIndex = 0; texelIndex < texelCount; texelIndex++)
			{
				m_rgbaTexels[texelIndex].r = *tempImageData;
				tempImageData++;
				m_rgbaTexels[texelIndex].g = *tempImageData;
				tempImageData++;
				m_rgbaTexels[texelIndex].b = *tempImageData;
				tempImageData++;
				m_rgbaTexels[texelIndex].a = 255;
			}
		}
		else if (numColorChannels == 4)
		{
			m_rgbaTexels.resize(texelCount);
			memcpy(m_rgbaTexels.data(), imageData, texelCount * sizeof(Rgba8));
		}

		stbi_image_free(imageData);
	}
	else
	{
		ERROR_AND_DIE("Failed to load the image");
	}
}

Image::Image(IntVec2 size, Rgba8 color)
	:m_dimensions(size)
{
	size_t texelCount = static_cast<size_t>(size.x) * static_cast<size_t>(size.y);
	m_rgbaTexels.resize(texelCount, color);
}

Image* Image::CreateDefaultTexture()
{
	Image* newImage;
	newImage = new Image(IntVec2(2, 2), Rgba8::WHITE);
	newImage->m_imageFilePath = "Default";
	return newImage;
}

const void* Image::GetRawData() const
{
	return reinterpret_cast<const void*>(m_rgbaTexels.data());
}

std::string const& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

Rgba8 Image::GetTexelColor(IntVec2 const& texelCoords) const
{
	// Check bounds to ensure texel coords are within the image dimensions
	int index = (texelCoords.y * m_dimensions.x) + texelCoords.x;
	return m_rgbaTexels[index];
}

void Image::SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor)
{
	// Check bounds to ensure texel coords are within the image dimensions
	int index = (texelCoords.y * m_dimensions.x) + texelCoords.x;
	m_rgbaTexels[index] = newColor;
}
