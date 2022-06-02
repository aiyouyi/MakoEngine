#include "EllipseDeformation.h"


EllipseDeformation::EllipseDeformation()
{
	m_pShiftRGBA = NULL;
	m_pOffestTexture = NULL;
}

EllipseDeformation::~EllipseDeformation()
{
	SAFE_DELETE_ARRAY(m_pShiftRGBA);
	SAFE_DELETE_ARRAY(m_pOffestTexture);
}




void EllipseDeformation::SetEllipseControlPoint(EllipseRect &rect, bool iSymmetry)
{

	m_EllipseRect = rect;

	m_iSymmetry = iSymmetry;


}

void EllipseDeformation::updateOffset()
{

	if (m_pShiftRGBA == NULL)
	{
		m_pShiftRGBA = new Vector2[m_SmallTextureHeight * m_SmallTextureWidth];
	}
	memset(m_pShiftRGBA, 0, sizeof(Vector2)*m_SmallTextureHeight * m_SmallTextureWidth);
	
	if (m_pOffestTexture == NULL)
	{
		m_pOffestTexture = new BYTE[m_SmallTextureHeight * m_SmallTextureWidth * 4];
	}
	Liquify();
	
	if (m_iSymmetry)
	{
		LiquifySymmetry();	
	}


	

	for (int y = 0; y < m_SmallTextureHeight; y++)
	{
		for (int x = 0; x < m_SmallTextureWidth; x++)
		{
			float xc = m_pShiftRGBA[(y*m_SmallTextureWidth + x)].x*255.f*4 + 127;
			float yc = m_pShiftRGBA[(y*m_SmallTextureWidth + x)].y*255.f*4 + 127;
			xc = (std::max)(0.f, (std::min)(xc, 255.f));
			yc = (std::max)(0.f, (std::min)(yc, 255.f));

			int xi = xc;
			int yi = yc;

			int g = (xc - xi) * 255;
			int a = (yc - yi) * 255;

			m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4] = xi;
			m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4 + 1] = g;
			m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4 + 2] = yi;
			m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4 + 3] = a;
		}
	}

}

BYTE * EllipseDeformation::GetOffsetTexture()
{
	if (!m_pOffestTexture)
	{
		return NULL;
	}
	return m_pOffestTexture;
}

void EllipseDeformation::Liquify()
{
	//***����80*80��ֱ�Ӵ�ƫ��ͼ�ᵼ����Բ�α䲻ƽ����
    //***����ӳ�䵽800*800�ڷֿ鵽80*80
	{
		int rect_left = (int)(m_EllipseRect.left * m_SmallTextureWidth * 10);
		int rect_top = (int)(m_EllipseRect.top * m_SmallTextureHeight * 10);
		int rect_width = (int)(m_EllipseRect.width  * m_SmallTextureWidth * 10);
		int rect_height = (int)(m_EllipseRect.height* m_SmallTextureHeight * 10);

		rect_left = (std::max)(0, (std::min)(rect_left, 800 - rect_width-1));
		rect_top = (std::max)(0, (std::min)(rect_top, 800 - rect_height-1));

		float a = rect_width / 2.0; // ���� �� ����
		float b = rect_height / 2.0;

		float middleX = rect_left + rect_width / 2.0;
		float middleY = rect_top + rect_height / 2.0;

		Vector2* pOffset = new Vector2[800 * 800];
		for (int i = rect_top; i < rect_top + rect_height; i++)
		{
			for (int j = rect_left; j < rect_left + rect_width; j++)
			{
				bool in_ellipse = InEllipse(j, i, rect_left, rect_top, rect_width, rect_height);
				if (in_ellipse)
				{
					// ��Բ���ĵ�(0,0)��(i,j)���ӳɵ�ֱ������Բ�Ľ���Ϊ��x,y��
					double ori_x = j - middleX; // ʹ�þֲ����꣬ (middleX, middleY)Ϊԭ��
					double ori_y = i - middleY; // �ֲ�����

					double k; // ԭ�����������ֱ�ߵ�б��
					double x, y;
					double ratio;
					double new_pos_ratioX;
					double new_pos_ratioY;
					double new_x;
					double new_y;

					if (fabs(ori_x) < 0.5)
					{
						new_x = 0;
						if (fabs(ori_y) < 0.5)
						{
							new_y = 0;
						}
						else if (ori_y > 0)
						{
							y = b;
							ratio = fabs(ori_y) / b;
							new_pos_ratioY = pow(ratio, m_LiquifyFactorY);
							new_y = y * new_pos_ratioY;
						}
						else if (ori_y < 0)
						{
							y = -b;
							ratio = fabs(ori_y) / b;
							new_pos_ratioY = pow(ratio, m_LiquifyFactorY);
							new_y = y * new_pos_ratioY;
						}
					}
					else
					{
						k = ori_y / ori_x;
						if (ori_x > 0)
						{
							x = a * b* sqrt(1.0 / (b * b + a * a*k*k));
						}
						else
						{
							x = -a * b* sqrt(1.0 / (b * b + a * a*k*k));
						}
						y = k * x;

						ratio = ori_x / x;
						if (ratio < 0)
						{
							LOGE("Error Ellipse Liquify...");
						}
						if (m_LiquifyFactorX >= 1)
						{
							new_pos_ratioX = pow(ratio, m_LiquifyFactorX);
						}
						else
						{
							new_pos_ratioX = ratio * (2 - m_LiquifyFactorX);
						}

						if (m_LiquifyFactorY >= 1)
						{
							new_pos_ratioY = pow(ratio, m_LiquifyFactorY);
						}
						else
						{
							new_pos_ratioY = ratio * (2 - m_LiquifyFactorY);
						}

						new_x = x * new_pos_ratioX;
						new_y = y * new_pos_ratioY;
					}

					float offset_x = new_x - ori_x;
					float offset_y = new_y - ori_y;

					pOffset[(i * 800 + j)].x = offset_x * m_LiquifyAlpha;
					pOffset[(i * 800 + j)].y = offset_y * m_LiquifyAlpha;

				}
			}
		}

		for (int m = 0; m < m_SmallTextureHeight; m++)
		{
			for (int n = 0; n < m_SmallTextureWidth; n++)
			{
				Vector2 sum_offset = Vector2(0.f, 0.f);
				for (int k = 0; k < 10; k++)
				{
					for (int j = 0; j < 10; j++)
					{
						sum_offset += pOffset[(10 * m + k) * 800 + (10 * n + j)];
					}
				}
				Vector2 offset_xy = sum_offset / 100.f;

				m_pShiftRGBA[(m*m_SmallTextureWidth + n)] = offset_xy * Vector2(1.f / 800, 1.f / 800);
			}
		}
		delete[]pOffset;
	}


	/*{
		int rect_left = (int)(m_EllipseRect.left * m_SmallTextureWidth);
		int rect_top = (int)(m_EllipseRect.top * m_SmallTextureHeight);
		int rect_width = (int)(m_EllipseRect.width  * m_SmallTextureWidth);
		int rect_height = (int)(m_EllipseRect.height* m_SmallTextureHeight);

		float a = (rect_width / 2.0)*10; // ���� �� ����
		float b = (rect_height / 2.0)*10;

		float middleX = (rect_left + rect_width / 2.0)*10;
		float middleY = (rect_top + rect_height / 2.0)*10;

		for (int i = rect_top; i < rect_top + rect_height; i++)
		{
			for (int j = rect_left; j < rect_left + rect_width; j++)
			{
				float sum_offset_x = 0.f;
				float sum_offset_y = 0.f;
				for (int m = 10 * i; m < 10 * (i + 1); m++)
				{
					for (int n = 10 * j; n < 10 * (j + 1); n++)
					{
						bool in_ellipse = InEllipse(n, m, rect_left*10, rect_top*10, rect_width*10, rect_height*10);

						if (in_ellipse)
						{
							// ��Բ���ĵ�(0,0)��(i,j)���ӳɵ�ֱ������Բ�Ľ���Ϊ��x,y��
							double ori_x = n - middleX; // ʹ�þֲ����꣬ (middleX, middleY)Ϊԭ��
							double ori_y = m - middleY; // �ֲ�����

							double k; // ԭ�����������ֱ�ߵ�б��
							double x, y;
							double ratio;
							double new_pos_ratioX;
							double new_pos_ratioY;
							double new_x;
							double new_y;

							if (fabs(ori_x) < 0.5)
							{
								new_x = 0;
								if (fabs(ori_y) < 0.5)
								{
									new_y = 0;
								}
								else if (ori_y > 0)
								{
									y = b;
									ratio = fabs(ori_y) / b;
									new_pos_ratioY = pow(ratio, m_LiquifyFactorY);
									new_y = y * new_pos_ratioY;
								}
								else if (ori_y < 0)
								{
									y = -b;
									ratio = fabs(ori_y) / b;
									new_pos_ratioY = pow(ratio, m_LiquifyFactorY);
									new_y = y * new_pos_ratioY;
								}
							}
							else
							{
								k = ori_y / ori_x;
								if (ori_x > 0)
								{
									x = a * b* sqrt(1.0 / (b * b + a * a*k*k));
								}
								else
								{
									x = -a * b* sqrt(1.0 / (b * b + a * a*k*k));
								}
								y = k * x;

								ratio = ori_x / x;
								if (ratio < 0)
								{
									LOGE("Error Ellipse Liquify...");
								}
								if (m_LiquifyFactorX >= 1)
								{
									new_pos_ratioX = pow(ratio, m_LiquifyFactorX);
								}
								else
								{
									new_pos_ratioX = ratio * (2 - m_LiquifyFactorX);
								}

								if (m_LiquifyFactorY >= 1)
								{
									new_pos_ratioY = pow(ratio, m_LiquifyFactorY);
								}
								else
								{
									new_pos_ratioY = ratio * (2 - m_LiquifyFactorY);
								}

								new_x = x * new_pos_ratioX;
								new_y = y * new_pos_ratioY;
							}

							float offset_x = new_x - ori_x;
							float offset_y = new_y - ori_y;
							sum_offset_x += offset_x;
							sum_offset_y += offset_y;

						}

					}
				}

				m_pShiftRGBA[i*m_SmallTextureHeight + j].x = sum_offset_x / 100.f *(1.f/800);
				m_pShiftRGBA[i*m_SmallTextureHeight + j].y = sum_offset_y / 100.f *(1.f/800);

			}
		}

	}*/

}

void EllipseDeformation::SetSymmetryAxis(float x)
{
	m_AxisX = x;
}

void EllipseDeformation::LiquifySymmetry()
{
	{
		float sym_left = 1.f - m_EllipseRect.left + (m_AxisX - 0.5)*0.2;
		int rect_left = (int)(sym_left * m_SmallTextureWidth * 10);
		int rect_top = (int)(m_EllipseRect.top * m_SmallTextureHeight * 10);
		int rect_width = (int)(m_EllipseRect.width  * m_SmallTextureWidth * 10);
		int rect_height = (int)(m_EllipseRect.height* m_SmallTextureHeight * 10);

		rect_left = (std::max)(0, (std::min)(rect_left, 800 - rect_width - 1));
		rect_top = (std::max)(0, (std::min)(rect_top, 800 - rect_height - 1));

		float a = rect_width / 2.0; // ���� �� ����
		float b = rect_height / 2.0;

		float middleX = rect_left - rect_width / 2.0;
		float middleY = rect_top + rect_height / 2.0;

		Vector2* pOffset = new Vector2[800 * 800];

		for (int i = rect_top; i < rect_top + rect_height; i++)
		{
			for (int j = rect_left; j > rect_left - rect_width; j--)
			{
				bool in_ellipse = InEllipse(j, i, rect_left - rect_width, rect_top, rect_width, rect_height);
				if (in_ellipse)
				{
					// ��Բ���ĵ�(0,0)��(i,j)���ӳɵ�ֱ������Բ�Ľ���Ϊ��x,y��
					double ori_x = j - middleX; // ʹ�þֲ����꣬ (middleX, middleY)Ϊԭ��
					double ori_y = i - middleY; // �ֲ�����

					double k; // ԭ�����������ֱ�ߵ�б��
					double x, y;
					double ratio;
					double new_pos_ratioX;
					double new_pos_ratioY;
					double new_x;
					double new_y;

					if (fabs(ori_x) < 0.5)
					{
						new_x = 0;
						if (fabs(ori_y) < 0.5)
						{
							new_y = 0;
						}
						else if (ori_y > 0)
						{
							y = b;
							ratio = fabs(ori_y) / b;
							new_pos_ratioY = pow(ratio, m_LiquifyFactorY);
							new_y = y * new_pos_ratioY;
						}
						else if (ori_y < 0)
						{
							y = -b;
							ratio = fabs(ori_y) / b;
							new_pos_ratioY = pow(ratio, m_LiquifyFactorY);
							new_y = y * new_pos_ratioY;
						}
					}
					else
					{
						k = ori_y / ori_x;
						if (ori_x > 0)
						{
							x = a * b* sqrt(1.0 / (b * b + a * a*k*k));
						}
						else
						{
							x = -a * b* sqrt(1.0 / (b * b + a * a*k*k));
						}
						y = k * x;

						ratio = ori_x / x;
						if (ratio < 0)
						{
							LOGE("Error Ellipse Liquify...");
						}
						if (m_LiquifyFactorX >= 1)
						{
							new_pos_ratioX = pow(ratio, m_LiquifyFactorX);
						}
						else
						{
							new_pos_ratioX = ratio * (2 - m_LiquifyFactorX);
						}

						if (m_LiquifyFactorY >= 1)
						{
							new_pos_ratioY = pow(ratio, m_LiquifyFactorY);
						}
						else
						{
							new_pos_ratioY = ratio * (2 - m_LiquifyFactorY);
						}

						new_x = x * new_pos_ratioX;
						new_y = y * new_pos_ratioY;
					}

					float offset_x = new_x - ori_x;
					float offset_y = new_y - ori_y;

					pOffset[(i * 800 + j)].x = offset_x * m_LiquifyAlpha;
					pOffset[(i * 800 + j)].y = offset_y * m_LiquifyAlpha;

				}
			}
		}

		for (int m = 0; m < m_SmallTextureHeight; m++)
		{
			for (int n = 0; n < m_SmallTextureWidth; n++)
			{
				Vector2 sum_offset = Vector2(0.f, 0.f);
				for (int k = 0; k < 10; k++)
				{
					for (int j = 0; j < 10; j++)
					{
						sum_offset += pOffset[(10 * m + k) * 800 + (10 * n + j)];
					}
				}
				Vector2 offset_xy = sum_offset / 100.f;

				m_pShiftRGBA[(m*m_SmallTextureWidth + n)] += offset_xy * Vector2(1.f / 800, 1.f / 800);
			}
		}
		delete[]pOffset;
	}
}

bool EllipseDeformation::InEllipse(float x, float y, float ellipse_left, float ellipse_top, float ellipse_width, float ellipse_height)
{
	float a = ellipse_width / 2;	// ����
	float b = ellipse_height / 2;	// ����
	float rel_x = x - (ellipse_left + a);
	float rel_y = y - (ellipse_top + b);
	if ((rel_x*rel_x) / (a*a) + (rel_y*rel_y) / (b*b) <= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void EllipseDeformation::SetLiquifyFactorX(float &xRadius)
{
	m_LiquifyFactorX = xRadius;
}

void EllipseDeformation::SetLiquifyFactorY(float &yRadius)
{
	m_LiquifyFactorY = yRadius;
}

void EllipseDeformation::ReadConfig(XMLNode &childNode)
{
	XMLNode nodeDrawable = childNode.getChildNode("ellipseInfo", 0);

	if (!nodeDrawable.isEmpty())
	{
		const char *szRect = nodeDrawable.getAttribute("rect");

		if (szRect != NULL)
		{
			float left = 0.f;
			float top = 0.f;
			float width = 0.f;
			float height = 0.f;
			sscanf(szRect, "%f,%f,%f,%f", &left, &top, &width, &height);
			m_EllipseRect.left = left;
			m_EllipseRect.top = top;
			m_EllipseRect.width = width;
			m_EllipseRect.height = height;
		}

		const char *szAxis = nodeDrawable.getAttribute("anis");
		if (szAxis != NULL)
		{
			sscanf(szAxis, "%f", &m_AxisX);
		}

		const char *szFactor = nodeDrawable.getAttribute("factor");
		if (szFactor != NULL)
		{
			sscanf(szFactor, "%f,%f", &m_LiquifyFactorX, &m_LiquifyFactorY);
		}

		const char *szSym = nodeDrawable.getAttribute("symmetry");
		if (szSym != NULL && !strcmp(szSym, "true"))
		{
			m_iSymmetry = true;
		}
		else
		{
			m_iSymmetry = false;
		}

	}

	updateOffset();
}

void EllipseDeformation::WriteConfig(XMLNode &nodeEffect)
{
	XMLNode nodeDrwable = nodeEffect.addChild("ellipseinfo");
	char rect[256];
	sprintf(rect, "%.4f,%.4f,%.4f,%.4f", m_EllipseRect.left, m_EllipseRect.top,m_EllipseRect.width, m_EllipseRect.height);
    nodeDrwable.addAttribute("rect", rect);

	char factor[128];

	sprintf(factor, "%.4f", m_AxisX);
	nodeDrwable.addAttribute("anis", factor);

	sprintf(factor, "%.4f,%.4f", m_LiquifyFactorX, m_LiquifyFactorY);
	nodeDrwable.addAttribute("factor", factor);

	if (m_iSymmetry)
	{
		nodeDrwable.addAttribute("symmetry", "true");
	}
	else
	{
		nodeDrwable.addAttribute("symmetry", "false");
	}
	

}