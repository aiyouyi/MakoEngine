#pragma once
#ifndef _H_MTMASKFILLUTIL_H_
#define _H_MTMASKFILLUTIL_H_
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include <algorithm>
using namespace std;

	/*
	 * 有效值范围在0-255,-1为无效值
	 */
	typedef int MTFillColor;
	/*
	 * 无效颜色值
	 */
#ifndef MT_INVALID_COLOR
#define MT_INVALID_COLOR -1
#endif

	 /*
	  * 多边形类型
	  */
	enum MTPolygonType {
		MT_ConcavePolygon, // （完全）凹多边形
		MT_HorizontalBinaryPolygon, // 可水平二分查找的多边形
		MT_VerticalBinaryPolygon, // 可竖直二分查找的多边形
		MT_ConvexPolygon // （完全）凸多边形
	};

	/****************************************************************
	 * 文件工具类 MTFileUtil
	 * @author:		yzm
	 *
	 * 提供Mask填充功能
	 ****************************************************************/
	class  CMaskFillUtil {
	public:

	public:
		/*
		 * 填充Mask
		 * @param pMask					Mask字节流
		 * @param iWidth				Mask的宽
		 * @param iHeight				Mask的高
		 * @param pPoints				多边形点集
		 * @param iCount				多边形点的数量
		 * @param polygonType			多边形的类型,如果是凸多边形会加速算法计算
		 * @param inside				多边形内部填充颜色（-1为无效值，表示保留原色）
		 * @param outside				多边形外部（Mask内部）填充颜色（-1为无效值，表示保留原色）
		 * @return						填充是否成功
		 */
		static bool FillMask(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTPolygonType polygonType, MTFillColor inside, MTFillColor outside);

	protected:
		/*
		 * 填充Mask（直接填充）
		 * @param pMask					Mask字节流
		 * @param iWidth				Mask的宽
		 * @param iHeight				Mask的高
		 * @param pPoints				多边形点集
		 * @param iCount				多边形点的数量
		 * @param inside				多边形内部填充颜色（-1为无效值，表示保留原色）
		 * @param outside				多边形外部（Mask内部）填充颜色（-1为无效值，表示保留原色）
		 * @return						填充是否成功
		 */
		static bool DirectFillPolygon(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor inside, MTFillColor outside);
		/*
		 * 填充Mask内除多边形部分（直接填充）
		 * @param pMask					Mask字节流
		 * @param iWidth				Mask的宽
		 * @param iHeight				Mask的高
		 * @param pPoints				多边形点集
		 * @param iCount				多边形点的数量
		 * @param fillColor				多边形外部（Mask内部）填充颜色（多边形内部保持原色）
		 * @return						填充是否成功
		 */
		static bool DirectFillPolygonOutside(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor fillColor);
		/*
		 * 填充Mask内部的多边形（垂直填充Mask，要求多边形必须是水平凸多边形（即左右边不存在大于180度的内角））
		 * @param pMask					Mask字节流
		 * @param iWidth				Mask的宽
		 * @param iHeight				Mask的高
		 * @param pPoints				多边形点集
		 * @param iCount				多边形点的数量
		 * @param inside				多边形内部填充颜色（-1为无效值，表示保留原色）
		 * @param outside				多边形外部（Mask内部）填充颜色（-1为无效值，表示保留原色）
		 * @return						填充是否成功
		 */
		static bool VerticalFillPolygon(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor inside, MTFillColor outside);
		/*
		 * 填充Mask内除多边形部分（垂直填充Mask，要求多边形必须是水平凸多边形（即左右边不存在大于180度的内角））
		 * @param pMask				Mask字节流
		 * @param iWidth				Mask的宽
		 * @param iHeight				Mask的高
		 * @param pPoints				多边形点集
		 * @param iCount				多边形点的数量
		 * @param fillColor			多边形外部（Mask内部）填充颜色（多边形内部保持原色）
		 * @return						填充是否成功
		 */
		static bool VerticalFillPolygonOutside(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor fillColor);
		/*
		 * 填充Mask内部的多边形（水平填充Mask，要求多边形必须是垂直凸多边形（即上下边不存在大于180度的内角））
		 * @param pMask					Mask字节流
		 * @param iWidth				Mask的宽
		 * @param iHeight				Mask的高
		 * @param pPoints				多边形点集
		 * @param iCount				多边形点的数量
		 * @param inside				多边形内部填充颜色（-1为无效值，表示保留原色）
		 * @param outside				多边形外部（Mask内部）填充颜色（-1为无效值，表示保留原色）
		 * @return						填充是否成功
		 */
		static bool HorizontalFillPolygon(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor inside, MTFillColor outside);
		/*
		 * 填充Mask内除多边形部分（水平填充Mask，要求多边形必须是垂直凸多边形（即上下边不存在大于180度的内角））
		 * @param pMask					Mask字节流
		 * @param iWidth				Mask的宽
		 * @param iHeight				Mask的高
		 * @param pPoints				多边形点集
		 * @param iCount				多边形点的数量
		 * @param fillColor				多边形外部（Mask内部）填充颜色（多边形内部保持原色）
		 * @return						填充是否成功
		 */
		static bool HorizontalFillPolygonOutside(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor fillColor);
		/*
		 * 判断点是否在多边形内部
		 * @param pPolygon				多边形点集
		 * @param iCount				多边形点的数量
		 * @param point					点坐标
		 * @return						点是否在多边形内部
		 */
		static bool InsidePolygon(const Vector2 *pPolygon, int iCount, Vector2 point);

	protected:
		/*
		 * 矩形
		 */
		struct MTRect {
		public:
			int iLeftX;
			int iLeftY;
			int iRightX;
			int iRightY;
		};

		/*
		 * 求多边形的外接矩形
		 * @param pPolygon				多边形点集（其中pPolygon[0] == pPolygon[iCount]）
		 * @param iCount				多边形点的数量
		 * @param point					点坐标
		 * @return						点是否在多边形内部
		 */
		static MTRect OutsideRect(const Vector2 *pPolygon, int iCount, int iBoderX, int iBorderY);
		/*
		 * 求环形点集
		 * 注：pNewPoints = pPoints; pNewPoints[iCount] = pNewPoint[0];
		 * @param pPoints				点集
		 * @param iCount				点的数量
		 * @return						环形点集（其中pPolygon[0] == pPolygon[iCount]）
		 */
		static Vector2* GetCirclePoints(const Vector2 *pPoints, int iCount);

	protected:
		/*
		 * 区间
		 */
		struct MTRange {
		public:
			MTRange(int iStart, int iEnd) : m_iStart(iStart), m_iEnd(iEnd) {}
		public:
			int m_iStart;
			int m_iEnd;
		};

		/*
		 * 对(x, y)在多边形pPolygon内时，求y的取值范围
		 * @param pPolygon				多边形点集（其中pPolygon[0] == pPolygon[iCount]）
		 * @param iCount				多边形点的数量
		 * @param range					y坐标的预取值范围
		 * @param iX					固定x坐标，即x = iX
		 * @return						y坐标的取值范围，当range.m_iStart > range.m_iEnd,及说明当x = iX时,不存在(x, y)在多边形pPolygon内
		 */
		static MTRange VerticalDichotomize(const Vector2 *pPolygon, int iCount, const MTRange& range, int iX);
		/*
		 * 对(x, y)在多边形pPolygon内时，求x的取值范围
		 * @param pPolygon				多边形点集（其中pPolygon[0] == pPolygon[iCount]）
		 * @param iCount				多边形点的数量
		 * @param range					x坐标的预取值范围
		 * @param Y						固定y坐标,即y = iY
		 * @return						x坐标的取值范围，当range.m_iStart > range.m_iEnd,及说明当x = iX时,不存在(x, y)在多边形pPolygon内
		 */
		static MTRange HorizontalDichotomize(const Vector2 *pPolygon, int iCount, const MTRange& range, int iY);
	};

#endif