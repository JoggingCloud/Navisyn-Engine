#pragma once
#include <string>

struct Rgba8
{
	Rgba8();
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

	explicit Rgba8(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = 255);

	void GetAsFloats(float* colorAsFloats) const;
	void SetFromText(char const* text);
	Rgba8 SetFromString(const std::string& colorName);
	Rgba8 GetLighterColor(float fraction) const;
	Rgba8 GetDarkerColor(float fraction) const;
	static Rgba8 GetRandomShadeOfBlue();
	static Rgba8 GetRandomColor(unsigned char a = 255);
	static Rgba8 GetColorForIndex(size_t index);

	static const Rgba8 NO_COLOR;
	static const Rgba8 SHADOW_BLACK;
	static const Rgba8 TRANSLUCENT_BLACK;
	static const Rgba8 MORE_TRANSLUCENT;
	static const Rgba8 TRANSLUCENT_YELLOW;
	static const Rgba8 TRANSLUCENT_GREEN;
	static const Rgba8 TRANSLUCENT_RED;
	static const Rgba8 TRANSLUCENT;
	static const Rgba8 WHITE;
	static const Rgba8 DARK_RED;
	static const Rgba8 RED;
	static const Rgba8 LIGHT_RED;
	static const Rgba8 DARK_GREEN;
	static const Rgba8 GREEN;
	static const Rgba8 LIGHT_GREEN;
	static const Rgba8 DARK_BLUE;
	static const Rgba8 BLUE;
	static const Rgba8 LIGHT_BLUE;
	static const Rgba8 BLACK;
	static const Rgba8 BROWN;
	static const Rgba8 DARK_ORANGE;
	static const Rgba8 LIGHT_ORANGE;
	static const Rgba8 YELLOW;
	static const Rgba8 PURPLE;
	static const Rgba8 DARK_GRAY;
	static const Rgba8 LIGHT_GRAY;
	static const Rgba8 AQUA;
	static const Rgba8 MAGENTA;
	static const Rgba8 CYAN;
	static const Rgba8 VIOLET;
	static const Rgba8 INDIGO;
	static const Rgba8 TEAL;
	static const Rgba8 CORAL;
	static const Rgba8 SALMON;
	static const Rgba8 TAN;
	static const Rgba8 LAVENDER;
	static const Rgba8 TURQUOISE;
	static const Rgba8 IVORY;
	static const Rgba8 BEIGE;
	static const Rgba8 PEACH;
	static const Rgba8 GOLD;
	static const Rgba8 SILVER;
	static const Rgba8 BRONZE;
	static const Rgba8 OLIVE;
	static const Rgba8 MAROON;
	static const Rgba8 NAVY;
	static const Rgba8 MINT;
	static const Rgba8 FUCHSIA;
	static const Rgba8 CRIMSON;
	static const Rgba8 AMBER;
	static const Rgba8 LIME;
	static const Rgba8 CHARTREUSE;
	static const Rgba8 KHAKI;
	static const Rgba8 RUBY;
	static const Rgba8 JADE;
	static const Rgba8 ROSE;
	static const Rgba8 EMERALD;
	static const Rgba8 SAPPHIRE;
	static const Rgba8 LEMON;
	static const Rgba8 PINK;
	static const Rgba8 HOT_PINK;
	static const Rgba8 PLUM;
	static const Rgba8 PERIWINKLE;
	static const Rgba8 SKY_BLUE;
	static const Rgba8 SEA_GREEN;
	static const Rgba8 SLATE_GRAY;
	static const Rgba8 SPRING_GREEN;
	static const Rgba8 WISTERIA;
	static const Rgba8 ALMOND; 
	static const Rgba8 APRICOT;
	static const Rgba8 AZURE;
	static const Rgba8 BABY_BLUE;
	static const Rgba8 BANANA_YELLOW;
	static const Rgba8 BUBBLEGUM_PINK;
	static const Rgba8 BYZANTIUM;
	static const Rgba8 CANDY_APPLE_RED;
	static const Rgba8 CARIBBEAN_GREEN;
	static const Rgba8 CELADON;
	static const Rgba8 CERULEAN;
	static const Rgba8 CHERRY_BLOSSOM_PINK;
	static const Rgba8 CINNAMON;
	static const Rgba8 COFFEE;
	static const Rgba8 CREAM;
	static const Rgba8 DENIM;
	static const Rgba8 EGGPLANT;
	static const Rgba8 ELECTRIC_BLUE;
	static const Rgba8 ELECTRIC_BLUE_LIGHT;
	static const Rgba8 FIREBRICK;
	static const Rgba8 FLAMINGO_PINK;
	static const Rgba8 FOREST_GREEN ;
	static const Rgba8 FROST;
	static const Rgba8 GINGER;
	static const Rgba8 HONEY;
	static const Rgba8 IRIS;
	static const Rgba8 LAVENDER_BLUSH ;
	static const Rgba8 LILAC;
	static const Rgba8 LION;
	static const Rgba8 MALACHITE;
	static const Rgba8 MOCHA;
	static const Rgba8 MULBERRY;
	static const Rgba8 OCHRE;
	static const Rgba8 ORCHID;
	static const Rgba8 PEAR;
	static const Rgba8 PERSIAN_BLUE;
	static const Rgba8 PINE_GREEN;
	static const Rgba8 POPPY_RED;
	static const Rgba8 PRUSSIAN_BLUE;
	static const Rgba8 RASPBERRY;
	static const Rgba8 RUST;
	static const Rgba8 SAND;
	static const Rgba8 SCARLET;
	static const Rgba8 SEAFOAM_GREEN;
	static const Rgba8 SEPIA;


	bool		operator==( const Rgba8& compare ) const;
	bool        operator<(const Rgba8& compare) const;
	bool		operator!=(const Rgba8& compare) const;
};

namespace std
{
	template <>
	struct hash<Rgba8>
	{
		std::size_t operator()(const Rgba8& color) const noexcept
		{
			std::size_t h1 = std::hash<float>{}(color.r);
			std::size_t h2 = std::hash<float>{}(color.g);
			std::size_t h3 = std::hash<float>{}(color.b);
			std::size_t h4 = std::hash<float>{}(color.a);
			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
		}
	};
}
