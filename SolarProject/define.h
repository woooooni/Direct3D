#pragma once

#define SINGLETON(type) public:\
						static type* GetInst()\
						{\
							static type manager;\
							return &manager;\
						}\
						private:\
							type();\
							~type();

#define FULL_SCREEN false
#define VSYNC_ENABLED false
#define SCREEN_DEPTH 1000.0f
#define SCREEN_NEAR 0.1f
#define Safe_Release(type) if(nullptr != type) type->Release(); type = 0;

enum class BRUSH_TYPE
{
	HOLLOW,
	END,
};

enum class PEN_TYPE
{
	RED,
	GREEN,
	BLUE,
	END,
};
