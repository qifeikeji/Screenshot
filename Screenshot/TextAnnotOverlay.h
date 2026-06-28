#pragma once

#include <vector>

struct TextAnnotBlock
{
	CRect rect;
	CString text;
};

class CTextAnnotOverlay
{
public:
	void Clear();
	size_t Count() const { return m_blocks.size(); }
	TextAnnotBlock* At(size_t index);
	const TextAnnotBlock* At(size_t index) const;

	int HitTest(CPoint clientPt) const;
	int HitTestDragHandle(CPoint clientPt) const;
	static CRect DragHandleRect(const CRect& boxRect);
	void AddBlock(const CRect& clientRect, LPCTSTR text);
	CRect MakeCenteredBox(const CRect& selectionClient, size_t stackIndex) const;
	CRect MakeBoxAtPoint(CPoint clientPt, size_t stackIndex) const;
	void MeasureAndResizeBlock(TextAnnotBlock& block, int maxWrapWidth, CFont* pFont) const;

	std::vector<TextAnnotBlock> GetBlocksCopy() const;
	void SetBlocks(const std::vector<TextAnnotBlock>& blocks);

	void DrawAll(CDC& dc, int skipEditIndex, CFont* pFont, COLORREF borderColor, COLORREF textColor) const;
	void DrawAllOnHDC(HDC hdc, CFont* pFont, COLORREF borderColor, COLORREF textColor) const;

private:
	std::vector<TextAnnotBlock> m_blocks;
};
