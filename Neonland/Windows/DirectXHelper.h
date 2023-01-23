#pragma once

namespace DX
{
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static constexpr float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f);
	}
}
