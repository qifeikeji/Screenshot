#pragma once

// ѡ���ڵı�ע�㣨32 λ ARGB�������ͼ�ϳɺ󵼳���
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

	// ����ע�㰴 Alpha ��ϻ��Ƶ�Ŀ�� DC��Ŀ�����Ͻ� destX, destY��
	void DrawOn(HDC hdcDest, int destX, int destY) const;

private:
	void ReleaseBitmap();
	HBITMAP CreateTransparentBitmap(int cx, int cy) const;

	HBITMAP m_hBitmap;
	CSize m_size;
};
