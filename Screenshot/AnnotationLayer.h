#pragma once

#include <vector>

class CAnnotationLayer
{
public:
	CAnnotationLayer();
	~CAnnotationLayer();

	void Clear();
	void EnsureSize(int cx, int cy);
	bool IsValid() const;
	bool HasVisibleContent() const;

	HBITMAP GetBitmap() const { return m_hBitmap; }
	CSize GetSize() const { return m_size; }

	void DrawOn(HDC hdcDest, int destX, int destY) const;

	void PushUndoSnapshot();
	BOOL Undo();

	void DrawLine(int x1, int y1, int x2, int y2, int width, COLORREF color);
	void DrawRectangle(int x1, int y1, int x2, int y2, int width, COLORREF color, BOOL filled);
	void DrawEllipse(int x1, int y1, int x2, int y2, int width, COLORREF color, BOOL filled);
	void DrawArrow(int x1, int y1, int x2, int y2, int width, COLORREF color);
	void DrawTextAt(int x, int y, LPCTSTR text, int height, COLORREF color);

	HBITMAP CopySnapshot() const;
	void ApplySnapshot(HBITMAP hb, int cx, int cy);

private:
	void FixAlphaChannel() const;
	void RefreshBitsPointer();
	void ReleaseBitmap();
	HBITMAP CreateTransparentBitmap(int cx, int cy);
	HBITMAP CloneBitmap(HBITMAP src) const;
	HDC BeginDraw(HGDIOBJ* outOld);
	void EndDraw(HDC hdc, HGDIOBJ old);

	HBITMAP m_hBitmap;
	CSize m_size;
	mutable void* m_pBits;
	std::vector<HBITMAP> m_undoStack;
	static const size_t kMaxUndo = 24;
};
