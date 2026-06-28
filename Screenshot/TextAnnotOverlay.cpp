#include "stdafx.h"
#include "TextAnnotOverlay.h"

namespace {

const int kMinBoxW = 72;
const int kMinBoxH = 28;
const int kTextPad = 6;
const int kDefaultBoxW = 120;
const int kDefaultBoxH = 32;

} // namespace

void CTextAnnotOverlay::Clear()
{
	m_blocks.clear();
}

TextAnnotBlock* CTextAnnotOverlay::At(size_t index)
{
	if (index >= m_blocks.size())
		return nullptr;
	return &m_blocks[index];
}

const TextAnnotBlock* CTextAnnotOverlay::At(size_t index) const
{
	if (index >= m_blocks.size())
		return nullptr;
	return &m_blocks[index];
}

int CTextAnnotOverlay::HitTest(CPoint clientPt) const
{
	for (int i = (int)m_blocks.size() - 1; i >= 0; --i)
	{
		if (DragHandleRect(m_blocks[(size_t)i].rect).PtInRect(clientPt))
			continue;
		CRect r = m_blocks[(size_t)i].rect;
		r.NormalizeRect();
		if (r.PtInRect(clientPt))
			return i;
	}
	return -1;
}

CRect CTextAnnotOverlay::DragHandleRect(const CRect& boxRect)
{
	CRect box = boxRect;
	box.NormalizeRect();
	const int cx = (box.left + box.right) / 2;
	const int cy = box.top - 8;
	return CRect(cx - 5, cy - 5, cx + 6, cy + 6);
}

int CTextAnnotOverlay::HitTestDragHandle(CPoint clientPt) const
{
	for (int i = (int)m_blocks.size() - 1; i >= 0; --i)
	{
		if (DragHandleRect(m_blocks[(size_t)i].rect).PtInRect(clientPt))
			return i;
	}
	return -1;
}

void CTextAnnotOverlay::AddBlock(const CRect& clientRect, LPCTSTR text)
{
	TextAnnotBlock b;
	b.rect = clientRect;
	b.text = text;
	m_blocks.push_back(b);
}

CRect CTextAnnotOverlay::MakeCenteredBox(const CRect& selectionClient, size_t stackIndex) const
{
	CRect sel = selectionClient;
	sel.NormalizeRect();
	const int off = (int)stackIndex * 14;
	CRect box(
		sel.left + (sel.Width() - kDefaultBoxW) / 2 + off,
		sel.top + (sel.Height() - kDefaultBoxH) / 2 + off,
		sel.left + (sel.Width() + kDefaultBoxW) / 2 + off,
		sel.top + (sel.Height() + kDefaultBoxH) / 2 + off);
	return box;
}

CRect CTextAnnotOverlay::MakeBoxAtPoint(CPoint clientPt, size_t stackIndex) const
{
	const int off = (int)stackIndex * 10;
	const int halfW = kDefaultBoxW / 2;
	const int halfH = kDefaultBoxH / 2;
	return CRect(
		clientPt.x - halfW + off,
		clientPt.y - halfH + off,
		clientPt.x + halfW + off,
		clientPt.y + halfH + off);
}

void CTextAnnotOverlay::MeasureAndResizeBlock(TextAnnotBlock& block, int maxWrapWidth, CFont* pFont) const
{
	if (maxWrapWidth < kMinBoxW + kTextPad * 2)
		maxWrapWidth = kMinBoxW + kTextPad * 2;

	CClientDC dc(nullptr);
	CFont* pOld = pFont ? dc.SelectObject(pFont) : nullptr;

	CRect calc(0, 0, maxWrapWidth - kTextPad * 2, 0);
	CString drawText = block.text;
	if (drawText.IsEmpty())
		drawText = _T(" ");
	UINT fmt = DT_LEFT | DT_TOP | DT_CALCRECT | DT_NOPREFIX | DT_EXPANDTABS;
	if (drawText.Find(_T('\n')) < 0)
		fmt |= DT_SINGLELINE;
	else
		fmt |= DT_WORDBREAK;
	dc.DrawText(drawText, calc, fmt);

	if (pOld)
		dc.SelectObject(pOld);

	int w = std::max(kMinBoxW, calc.Width() + kTextPad * 2);
	int h = std::max(kMinBoxH, calc.Height() + kTextPad * 2);

	if (w > maxWrapWidth)
	{
		w = maxWrapWidth;
		CRect calc2(0, 0, w - kTextPad * 2, 0);
		CClientDC dc2(nullptr);
		CFont* pOld2 = pFont ? dc2.SelectObject(pFont) : nullptr;
		dc2.DrawText(drawText, calc2, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX | DT_EXPANDTABS);
		if (pOld2)
			dc2.SelectObject(pOld2);
		h = std::max(kMinBoxH, calc2.Height() + kTextPad * 2);
	}

	CRect box = block.rect;
	box.NormalizeRect();
	const int left = box.left;
	const int top = box.top;
	block.rect.SetRect(left, top, left + w, top + h);
}

void CTextAnnotOverlay::DrawAll(CDC& dc, int skipEditIndex, CFont* pFont, COLORREF borderColor, COLORREF textColor) const
{
	DrawAllOnHDC(dc.m_hDC, pFont, borderColor, textColor);
	UNREFERENCED_PARAMETER(skipEditIndex);
}

void CTextAnnotOverlay::DrawAllOnHDC(HDC hdc, CFont* pFont, COLORREF borderColor, COLORREF textColor) const
{
	CPen pen(PS_SOLID, 2, borderColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, pen.GetSafeHandle());
	HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
	HGDIOBJ hOldFont = pFont ? SelectObject(hdc, pFont->GetSafeHandle()) : nullptr;
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, textColor);

	for (const TextAnnotBlock& b : m_blocks)
	{
		CRect r = b.rect;
		r.NormalizeRect();
		Rectangle(hdc, r.left, r.top, r.right, r.bottom);
		CRect textRc = r;
		textRc.DeflateRect(kTextPad, kTextPad);
		if (!b.text.IsEmpty())
			DrawTextW(hdc, b.text, b.text.GetLength(), textRc, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);
	}

	if (hOldFont)
		SelectObject(hdc, hOldFont);
	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
}
