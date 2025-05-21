#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"

Rgba8 const Rgba8::AQUA = Rgba8(154, 255, 251, 255);
Rgba8 const Rgba8::LIGHT_BLUE = Rgba8(45, 170, 214, 255);
Rgba8 const Rgba8::PURPLE = Rgba8(163, 73, 164, 255);
Rgba8 const Rgba8::TRANSLUCENT_YELLOW = Rgba8(255, 242, 0, 100);
Rgba8 const Rgba8::YELLOW = Rgba8(255, 242, 0, 255);
Rgba8 const Rgba8::LIGHT_ORANGE = Rgba8(255, 165, 0, 255);
Rgba8 const Rgba8::DARK_ORANGE = Rgba8(255, 127, 39, 255);
Rgba8 const Rgba8::TRANSLUCENT_RED = Rgba8(255, 0, 0, 100); 
Rgba8 const Rgba8::LIGHT_RED = Rgba8(255, 105, 105, 255);
Rgba8 const Rgba8::RED = Rgba8(255, 0, 0, 255);
Rgba8 const Rgba8::DARK_RED = Rgba8(136, 0, 21, 255);
Rgba8 const Rgba8::LIGHT_GREEN = Rgba8(150, 255, 146, 255);
Rgba8 const Rgba8::TRANSLUCENT_GREEN = Rgba8(0, 255, 0, 100);
Rgba8 const Rgba8::GREEN = Rgba8(0, 255, 0, 255);
Rgba8 const Rgba8::DARK_GREEN = Rgba8(24, 128, 55, 255);
Rgba8 const Rgba8::BLUE = Rgba8(0, 0, 255, 255);
Rgba8 const Rgba8::DARK_BLUE = Rgba8(0, 18, 154, 255);
Rgba8 const Rgba8::WHITE = Rgba8(255, 255, 255, 255);
Rgba8 const Rgba8::BLACK = Rgba8(0, 0, 0, 255);
Rgba8 const Rgba8::NO_COLOR = Rgba8(0, 0, 0, 0);
Rgba8 const Rgba8::SHADOW_BLACK = Rgba8(0, 0, 0, 100);
Rgba8 const Rgba8::TRANSLUCENT_BLACK = Rgba8(0, 0, 0, 25);
Rgba8 const Rgba8::BROWN = Rgba8(150, 75, 0, 255);
Rgba8 const Rgba8::DARK_GRAY = Rgba8(128, 128, 128, 255);
Rgba8 const Rgba8::LIGHT_GRAY = Rgba8(212, 212, 212, 255);
Rgba8 const Rgba8::TRANSLUCENT = Rgba8(255, 255, 255, 50);
Rgba8 const Rgba8::MORE_TRANSLUCENT = Rgba8(255, 255, 255, 150);
Rgba8 const Rgba8::MAGENTA = Rgba8(235, 72, 235, 255);
Rgba8 const Rgba8::CYAN = Rgba8(0, 255, 255, 255);
Rgba8 const Rgba8::VIOLET = Rgba8(148, 0, 211, 255);
Rgba8 const Rgba8::INDIGO = Rgba8(75, 0, 130, 255);
Rgba8 const Rgba8::TEAL = Rgba8(0, 128, 128, 255);
Rgba8 const Rgba8::CORAL = Rgba8(255, 127, 80, 255);
Rgba8 const Rgba8::SALMON = Rgba8(250, 128, 114, 255);
Rgba8 const Rgba8::TAN = Rgba8(210, 180, 140, 255);
Rgba8 const Rgba8::LAVENDER = Rgba8(230, 230, 250, 255);
Rgba8 const Rgba8::TURQUOISE = Rgba8(64, 224, 208, 255);
Rgba8 const Rgba8::IVORY = Rgba8(255, 255, 240, 255);
Rgba8 const Rgba8::BEIGE = Rgba8(245, 245, 220, 255);
Rgba8 const Rgba8::PEACH = Rgba8(255, 218, 185, 255);
Rgba8 const Rgba8::GOLD = Rgba8(255, 215, 0, 255);
Rgba8 const Rgba8::SILVER = Rgba8(192, 192, 192, 255);
Rgba8 const Rgba8::BRONZE = Rgba8(205, 127, 50, 255);
Rgba8 const Rgba8::OLIVE = Rgba8(128, 128, 0, 255);
Rgba8 const Rgba8::MAROON = Rgba8(128, 0, 0, 255);
Rgba8 const Rgba8::NAVY = Rgba8(0, 0, 128, 255);
Rgba8 const Rgba8::MINT = Rgba8(189, 252, 201, 255);
Rgba8 const Rgba8::FUCHSIA = Rgba8(255, 0, 255, 255);
Rgba8 const Rgba8::CRIMSON = Rgba8(220, 20, 60, 255);
Rgba8 const Rgba8::AMBER = Rgba8(255, 191, 0, 255);
Rgba8 const Rgba8::LIME = Rgba8(191, 255, 0, 255);
Rgba8 const Rgba8::CHARTREUSE = Rgba8(127, 255, 0, 255);
Rgba8 const Rgba8::KHAKI = Rgba8(240, 230, 140, 255);
Rgba8 const Rgba8::RUBY = Rgba8(224, 17, 95, 255);
Rgba8 const Rgba8::JADE = Rgba8(0, 168, 107, 255);
Rgba8 const Rgba8::ROSE = Rgba8(255, 228, 225, 255);
Rgba8 const Rgba8::EMERALD = Rgba8(80, 200, 120, 255);
Rgba8 const Rgba8::SAPPHIRE = Rgba8(15, 82, 186, 255);
Rgba8 const Rgba8::LEMON = Rgba8(255, 247, 0, 255);
Rgba8 const Rgba8::PINK = Rgba8(255, 192, 203, 255);
Rgba8 const Rgba8::HOT_PINK = Rgba8(255, 105, 180, 255);
Rgba8 const Rgba8::PLUM = Rgba8(221, 160, 221, 255);
Rgba8 const Rgba8::PERIWINKLE = Rgba8(204, 204, 255, 255);
Rgba8 const Rgba8::SKY_BLUE = Rgba8(135, 206, 235, 255);
Rgba8 const Rgba8::SEA_GREEN = Rgba8(46, 139, 87, 255);
Rgba8 const Rgba8::SLATE_GRAY = Rgba8(112, 128, 144, 255);
Rgba8 const Rgba8::SPRING_GREEN = Rgba8(0, 255, 127, 255);
Rgba8 const Rgba8::WISTERIA = Rgba8(201, 160, 220, 255);
Rgba8 const Rgba8::ALMOND = Rgba8(239, 222, 205, 255);
Rgba8 const Rgba8::APRICOT = Rgba8(251, 206, 177, 255);
Rgba8 const Rgba8::AZURE = Rgba8(240, 255, 255, 255);
Rgba8 const Rgba8::BABY_BLUE = Rgba8(137, 207, 240, 255);
Rgba8 const Rgba8::BANANA_YELLOW = Rgba8(255, 225, 53, 255);
Rgba8 const Rgba8::BUBBLEGUM_PINK = Rgba8(255, 193, 204, 255);
Rgba8 const Rgba8::BYZANTIUM = Rgba8(112, 41, 99, 255);
Rgba8 const Rgba8::CANDY_APPLE_RED = Rgba8(255, 8, 0, 255);
Rgba8 const Rgba8::CARIBBEAN_GREEN = Rgba8(0, 204, 153, 255);
Rgba8 const Rgba8::CELADON = Rgba8(172, 225, 175, 255);
Rgba8 const Rgba8::CERULEAN = Rgba8(0, 123, 167, 255);
Rgba8 const Rgba8::CHERRY_BLOSSOM_PINK = Rgba8(255, 183, 197, 255);
Rgba8 const Rgba8::CINNAMON = Rgba8(210, 105, 30, 255);
Rgba8 const Rgba8::COFFEE = Rgba8(111, 78, 55, 255);
Rgba8 const Rgba8::CREAM = Rgba8(255, 253, 208, 255);
Rgba8 const Rgba8::DENIM = Rgba8(21, 96, 189, 255);
Rgba8 const Rgba8::EGGPLANT = Rgba8(97, 64, 81, 255);
Rgba8 const Rgba8::ELECTRIC_BLUE = Rgba8(125, 249, 255, 255);
Rgba8 const Rgba8::ELECTRIC_BLUE_LIGHT = Rgba8(125, 249, 255, 75);
Rgba8 const Rgba8::FIREBRICK = Rgba8(178, 34, 34, 255);
Rgba8 const Rgba8::FLAMINGO_PINK = Rgba8(252, 142, 172, 255);
Rgba8 const Rgba8::FOREST_GREEN = Rgba8(34, 139, 34, 255);
Rgba8 const Rgba8::FROST = Rgba8(213, 242, 253, 255);
Rgba8 const Rgba8::GINGER = Rgba8(176, 101, 0, 255);
Rgba8 const Rgba8::HONEY = Rgba8(240, 200, 40, 255);
Rgba8 const Rgba8::IRIS = Rgba8(90, 79, 207, 255);
Rgba8 const Rgba8::LAVENDER_BLUSH = Rgba8(255, 240, 245, 255);
Rgba8 const Rgba8::LILAC = Rgba8(200, 162, 200, 255);
Rgba8 const Rgba8::LION = Rgba8(193, 154, 107, 255);
Rgba8 const Rgba8::MALACHITE = Rgba8(11, 218, 81, 255);
Rgba8 const Rgba8::MOCHA = Rgba8(158, 111, 72, 255);
Rgba8 const Rgba8::MULBERRY = Rgba8(197, 75, 140, 255);
Rgba8 const Rgba8::OCHRE = Rgba8(204, 119, 34, 255);
Rgba8 const Rgba8::ORCHID = Rgba8(218, 112, 214, 255);
Rgba8 const Rgba8::PEAR = Rgba8(209, 226, 49, 255);
Rgba8 const Rgba8::PERSIAN_BLUE = Rgba8(28, 57, 187, 255);
Rgba8 const Rgba8::PINE_GREEN = Rgba8(1, 121, 111, 255);
Rgba8 const Rgba8::POPPY_RED = Rgba8(237, 28, 36, 255);
Rgba8 const Rgba8::PRUSSIAN_BLUE = Rgba8(0, 49, 83, 255);
Rgba8 const Rgba8::RASPBERRY = Rgba8(227, 11, 92, 255);
Rgba8 const Rgba8::RUST = Rgba8(183, 65, 14, 255);
Rgba8 const Rgba8::SAND = Rgba8(194, 178, 128, 255);
Rgba8 const Rgba8::SCARLET = Rgba8(255, 36, 0, 255);
Rgba8 const Rgba8::SEAFOAM_GREEN = Rgba8(159, 226, 191, 255);
Rgba8 const Rgba8::SEPIA = Rgba8(112, 66, 20, 255);

bool Rgba8::operator<(const Rgba8& compare) const
{
	if (compare.r != r)
	{
		return compare.r < r;
	}
	if (compare.g != g)
	{
		return compare.g < g;
	}
	if (compare.b != b)
	{
		return compare.b < b;
	}
	return compare.a < a;
}

bool Rgba8::operator!=(const Rgba8& compare) const
{
	return compare.r != r || compare.g != g || compare.b != b || compare.a != a;
}

bool Rgba8::operator==(const Rgba8& compare) const
{
	return compare.r == r && compare.g == g && compare.b == b && compare.a == a;
}

Rgba8::Rgba8 (unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte)
	: r(redByte), g(greenByte), b(blueByte), a(alphaByte)
{
}

Rgba8::Rgba8()
{}

void Rgba8::GetAsFloats(float* colorAsFloats) const
{
	// Expect argument to be an array of 4 floats 
	if (colorAsFloats)
	{
		// Convert rgba from int to float between 0.0f-1.0f
		float rFloat = static_cast<float>(r) / 255.0f;
		float gFloat = static_cast<float>(g) / 255.0f;
		float bFloat = static_cast<float>(b) / 255.0f;
		float aFloat = static_cast<float>(a) / 255.0f;

		// Put them in an array in order
		colorAsFloats[0] = rFloat;
		colorAsFloats[1] = gFloat;
		colorAsFloats[2] = bFloat;
		colorAsFloats[3] = aFloat;
	}
	else
	{
		// Error Exception
	}
}

void Rgba8::SetFromText(char const* text)
{
	std::string originalText = text;

	// Split the text using commas 
	Strings components = SplitStringOnDelimiter(originalText, ',');

	size_t numComponents = components.size();

	if (numComponents == 2 || numComponents == 3)
	{
		// Parse red, green, and blue components 
		r = static_cast<unsigned char>(std::stoi(components[0].c_str()));
		g = static_cast<unsigned char>(std::stoi(components[1].c_str()));
		b = static_cast<unsigned char>(std::stoi(components[2].c_str()));

		// Parse alpha component if its present
		if (numComponents == 3)
		{
			a = 255;
		}
		else
		{
			a = static_cast<unsigned char>(std::stoi(components[3].c_str()));
		}
	}
	else
	{
		r = g = b = a = 255; // If incorrect num of components, set the default to 255
	}
}

Rgba8 Rgba8::SetFromString(const std::string& colorName)
{
	// Convert colorName to uppercase for case-insensitive comparison
	std::string uppercaseColorName = colorName;
	for (char& character : uppercaseColorName) // Iterates over each character using a range based for loop
	{
		character = static_cast<char>(std::toupper(character)); // toupper function takes a character and returns the uppercase version of that character if the character is not uppercase
	}

	if (uppercaseColorName == "WHITE") 
	{
		return Rgba8(255, 255, 255, 255);
	}
	else if (uppercaseColorName == "RED") 
	{
		return Rgba8(255, 0, 0, 255);
	}
	else if (uppercaseColorName == "GREEN") 
	{
		return Rgba8(0, 255, 0, 255);
	}
	else if (uppercaseColorName == "BLUE")
	{
		return Rgba8(0, 0, 255, 255);
	}
	else if (uppercaseColorName == "BLACK")
	{
		return Rgba8(0, 0, 0, 255);
	}
	else 
	{
		// For now, returning white as a default if color is unknown
		return Rgba8::WHITE;
	}
}

Rgba8 Rgba8::GetLighterColor(float fraction) const
{
	Rgba8 lighterColor;
	lighterColor.r = static_cast<uint8_t>(r + (255 - r) * fraction);
	lighterColor.g = static_cast<uint8_t>(g + (255 - g) * fraction);
	lighterColor.b = static_cast<uint8_t>(b + (255 - b) * fraction);
	lighterColor.a = a;
	return lighterColor;
}

Rgba8 Rgba8::GetDarkerColor(float fraction) const
{
	Rgba8 darkerColor;
	darkerColor.r = static_cast<uint8_t>(r * (1.f - fraction));
	darkerColor.g = static_cast<uint8_t>(g * (1.f - fraction));
	darkerColor.b = static_cast<uint8_t>(b * (1.f - fraction));
	darkerColor.a = a;
	return darkerColor;
}

Rgba8 Rgba8::GetRandomShadeOfBlue()
{
	unsigned char b = (unsigned char)g_rng.RollRandomIntInRange(180, 255);
	unsigned char g = (unsigned char)g_rng.RollRandomIntInRange(180, b);
	unsigned char r = (unsigned char)g_rng.RollRandomIntInRange(0, (int)g / 2);
	unsigned char a = 255;

	return Rgba8(r, g, b, a);
}

Rgba8 Rgba8::GetRandomColor(unsigned char a )
{
	unsigned char r = static_cast<unsigned char>(rand() % 256);
	unsigned char g = static_cast<unsigned char>(rand() % 256);
	unsigned char b = static_cast<unsigned char>(rand() % 256);
	
	return Rgba8(r, g, b, a);
}

Rgba8 Rgba8::GetColorForIndex(size_t index)
{
	unsigned char red = static_cast<unsigned char>((index * 53) % 256);  // Multiply by a prime number
	unsigned char green = static_cast<unsigned char>((index * 97) % 256);
	unsigned char blue = static_cast<unsigned char>((index * 149) % 256);
	return Rgba8(red, green, blue);
}
