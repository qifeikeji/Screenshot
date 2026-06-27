#pragma once

// 选区内的标注层（32 位 ARGB），与底图合成后导出。
class CAnnotationLayer
{
public:
	CAnnotationLayer();
	~CAnnotationLayer();

	void Clear();
	void EnsureSize(int cx, int cy);
	bool IsValid() const;

	HBITMAP GetBitmap() const { return m_hBitmap; }
	CSize GetSize() const { return m_size; }

	// 将标注层按 Alpha 混合绘制到目标 DC（目标左上角 destX, destY）
	void DrawOn(HDC hdcDest, int destX, int destY) const;

private:
	void ReleaseBitmap();
	HBITMAP CreateTransparentBitmap(int cx, int cy) const;

	HBITMAP m_hBitmap;
	CSize m_size;
};
