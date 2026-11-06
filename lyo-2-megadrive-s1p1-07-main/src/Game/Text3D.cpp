#include "pch.h"
#include "Text3D.h"

// Fonctions utiles

int Text3D::NextPow2(int value)
{
	int pow2 = 1;
	while (pow2 < value)
		pow2 <<= 1;
	return pow2;
}

// Transparence d'un pixel de la photo
static byte AtlasAlpha(const cpu_texture* pAtlas, int x, int y)
{
	if (x<0) x = 0; else if (x>=pAtlas->width) 
		x = pAtlas->width - 1;
	if (y<0) y = 0; else if (y>=pAtlas->height) 
		y = pAtlas->height - 1;
	return pAtlas->bgra[(y * pAtlas->width + x) * 4 + 3];
}

// True si un pixel plein touche (cx, cy) a distance r (perimetre du carre)
static bool AtlasRing(const cpu_texture* pAtlas, int cx, int cy, int r)
{
	for (int i=-r ; i<=r ; ++i)
	{
		if (AtlasAlpha(pAtlas, cx + i, cy - r)>=128) 
			return true;
		if (AtlasAlpha(pAtlas, cx + i, cy + r)>=128) 
			return true;
		if (AtlasAlpha(pAtlas, cx - r, cy + i)>=128) 
			return true;
		if (AtlasAlpha(pAtlas, cx + r, cy + i)>=128) 
			return true;
	}
	return false;
}

// Shader : jette les pixels vides de la photo, garde la lettre

void Text3D::GlyphPixelShader(cpu_ps_io& io)
{
	const cpu_texture* pAtlas = io.pMaterial->pTexture;
	if (pAtlas==nullptr || pAtlas->bgra==nullptr)
	{
		io.discard = true;
		return;
	}

	// Position -> pixel dans la photo
	const int cx = (int)(io.p.uv.x * (float)pAtlas->width);
	const int cy = (int)(io.p.uv.y * (float)pAtlas->height);

	const TextStyle* pStyle = (const TextStyle*)io.values;
	const bool bold = (pStyle!=nullptr && pStyle->bold);
	const int outline = (pStyle!=nullptr) ? pStyle->outlinePx : 0;

	// La lettre
	if (AtlasAlpha(pAtlas, cx, cy)>=(bold ? 64 : 128))
	{
		io.color = io.p.color;	// la lettre prend la couleur du texte
		return;
	}

	// Gras : on deborde d'1 pixel avec la couleur du texte
	if (bold && AtlasRing(pAtlas, cx, cy, 1))
	{
		io.color = io.p.color;
		return;
	}

	// Contour : on deborde avec la couleur du contour (au dela du gras)
	if (outline>0 && AtlasRing(pAtlas, cx, cy, outline))
	{
		io.color = pStyle->outlineColor;
		return;
	}

	io.discard = true;
}

// La photo des lettres doit avoir une taille puissance de 2
// (ex : 256, 512...). On la recopie dans une photo plus grande si besoin
// Le vide autour reste transparent

bool Text3D::CreateAtlas(int fontPx, cstr fontName)
{
	if (m_font.Create(fontPx, CPU_WHITE, fontName)==false)
		return false;
	if (m_font.bgra.empty())
		return false;

	const int width  = NextPow2(m_font.width);
	const int height = NextPow2(m_font.height);

	m_atlas.Close();	// jeter l'ancienne photo s'il y en a une
	m_atlas.bgra = new byte[width * height * 4]();	// vide = transparent
	m_atlas.width = width;
	m_atlas.height = height;
	m_atlas.count = width * height;
	m_atlas.size = width * height * 4;

	// Les lignes n'ont pas la meme largeur : on copie ligne par ligne
	for (int y=0 ; y<m_font.height ; ++y)
		memcpy(&m_atlas.bgra[y * width * 4], &m_font.bgra[y * m_font.width * 4], m_font.width * 4);

	return true;
}

// Demarrage : prepare la photo + la couleur de base (blanc)

bool Text3D::Init(int fontPx, cstr fontName)
{
	m_ready = false;

	if (CreateAtlas(fontPx, fontName)==false)
		return false;

	// Une lettre mesure 1.0 de haut
	m_cellWidth = (float)m_font.cellW / (float)m_font.cellH;

	m_glyphMaterial.pTexture = &m_atlas;
	m_glyphMaterial.lighting = CPU_LIGHTING_UNLIT;	// pas d'ombre sur le texte
	m_glyphMaterial.ps = &Text3D::GlyphPixelShader;
	m_flatMaterial.lighting = CPU_LIGHTING_UNLIT;	// bandes unies : pas d'ombre non plus

	m_ready = true;
	return true;
}

// Un rectangle par lettre, avec le bon morceau de photo

cpu_mesh* Text3D::GetGlyphMesh(char c, bool italic)
{
	const unsigned char index = (unsigned char)c;
	if (m_font.glyph[index].valid==false)
		return nullptr;

	// Droit et penche : deux rectangles differents
	const int key = ((int)index << 1) | (italic ? 1 : 0);
	const auto found = m_glyphs.find(key);
	if (found!=m_glyphs.end())
		return &found->second;

	const cpu_glyph& glyph = m_font.glyph[index];

	// Ou est la lettre dans la photo (haut = 0)
	const float invW = 1.0f / (float)m_atlas.width;
	const float invH = 1.0f / (float)m_atlas.height;
	const float u0 = glyph.x * invW;
	const float u1 = (glyph.x + glyph.w) * invW;
	const float v0 = glyph.y * invH;
	const float v1 = (glyph.y + glyph.h) * invH;

	// Le rectangle fait 1.0 de haut, il commence a gauche
	const float x1 = m_cellWidth;
	const float y0 = -0.5f;	// bas
	const float y1 =  0.5f;	// haut

	// Italique : le haut part a droite
	const float shear = italic ? 0.32f * 0.5f : 0.0f;
	XMFLOAT3 topLeft = { 0.0f + shear, y1, 0.0f };
	XMFLOAT3 topRight = { x1 + shear, y1, 0.0f };
	XMFLOAT3 bottomRight = { x1 - shear, y0, 0.0f };
	XMFLOAT3 bottomLeft = { 0.0f - shear, y0, 0.0f };

	XMFLOAT2 uvTopLeft = { u0, v0 };
	XMFLOAT2 uvTopRight = { u1, v0 };
	XMFLOAT2 uvBottomRight = { u1, v1 };
	XMFLOAT2 uvBottomLeft = { u0, v1 };

	XMFLOAT3 white = CPU_WHITE;

	cpu_mesh& mesh = m_glyphs[key];

	// Devant : ordre qui s'affiche a l'endroit
	mesh.AddFace(topLeft, topRight, bottomRight, bottomLeft, uvTopLeft, uvTopRight, uvBottomRight, uvBottomLeft, white);

	// Derriere : meme chose a l'envers, pour lire par transparence
	mesh.AddFace(bottomLeft, bottomRight, topRight, topLeft, uvBottomLeft, uvBottomRight, uvTopRight, uvTopLeft, white);

	// Obligatoire : sans ca le texte est invisible
	mesh.CalculateBoundingVolumes();

	return &mesh;
}

float Text3D::GetAdvance(char) const
{
	// Toutes les lettres prennent la meme place
	return m_cellWidth;
}

// Assemble une lettre : rectangle + couleur + position + orientation

cpu_entity* Text3D::CreateLetter(char c, cpu_material* pMaterial, XMFLOAT3 pos, float size, const XMFLOAT3& right, const XMFLOAT3& up, const XMFLOAT3& dir)
{
	cpu_mesh* pMesh = GetGlyphMesh(c, m_italic);
	if (pMesh==nullptr)
		return nullptr;

	cpu_entity* pEntity = cpuEngine.CreateEntity();
	pEntity->pMesh = pMesh;
	pEntity->pMaterial = pMaterial;
	pEntity->transform.Identity();	// on repart de zero
	pEntity->transform.SetPosition(pos.x, pos.y, pos.z);
	pEntity->transform.SetScaling(size);

	// On applique l'orientation
	pEntity->transform.right = right;
	pEntity->transform.up = up;
	pEntity->transform.dir = dir;
	pEntity->transform.SetRotationFromAxes();

	m_entities.push_back(pEntity);
	return pEntity;
}

// Construit tout le texte lettre par lettre

std::vector<cpu_entity*> Text3D::Build(cstr text, XMFLOAT3 origin, float size, XMFLOAT3 color)
{
	std::vector<cpu_entity*> created;

	if (m_ready==false || text==nullptr)
		return created;

	Release();	// un seul texte a la fois : le precedent est jete
	m_builtColor = color;
	m_origin = origin;	// pivot du groupe pour SetPosition / Move / AddYPR

	// Une couleur pour tout le texte
	cpu_material* pMaterial = new cpu_material(m_glyphMaterial);
	pMaterial->color = color;
	m_materials.push_back(pMaterial);

	// Style du texte, lu par le shader (detruit au Release)
	m_styles.push_back(std::unique_ptr<TextStyle>(new TextStyle()));
	TextStyle* pStyle = m_styles.back().get();
	pStyle->outlineColor = m_outlineColor;
	pStyle->outlinePx = (int)(m_outlinePx + 0.5f);
	pStyle->bold = m_bold;
	pMaterial->values = pStyle;

	// Orientation memorisee (debout, face -Z, au premier Build ; garde la rotation ensuite)
	const XMFLOAT3 right = m_right;
	const XMFLOAT3 up = m_up;
	const XMFLOAT3 dir = m_dir;

	float pen = 0.0f;		// ou on est dans la ligne
	float maxPen = 0.0f;	// ligne la plus large (pour l'ancre)
	int line = 0;			// numero de ligne (apres \n)
	int firstOfLine = -1;	// 1re lettre de la ligne (pour le souligne)

	for (cstr p=text ; *p!='\0' ; ++p)
	{
		if (*p=='\n')
		{
			if ( pen>maxPen )
				maxPen = pen;
			m_lineW.push_back(pen);
			m_lineFirst.push_back(firstOfLine);
			firstOfLine = -1;
			pen = 0.0f;
			++line;
			continue;
		}

		// Disposition dans le repere du groupe (suit la rotation)
		XMFLOAT3 pos = { origin.x + right.x * pen * size - up.x * (float)line * size,
		                 origin.y + right.y * pen * size - up.y * (float)line * size,
		                 origin.z + right.z * pen * size - up.z * (float)line * size };
		cpu_entity* pEntity = CreateLetter(*p, pMaterial, pos, size, right, up, dir);
		if (pEntity!=nullptr)
		{
			created.push_back(pEntity);
			m_chars.push_back(*p);
			if (firstOfLine<0)
				firstOfLine = (int)m_chars.size() - 1;
		}

		pen += GetAdvance(*p);
	}
	if (pen>maxPen)
		maxPen = pen;
	m_lineW.push_back(pen);
	m_lineFirst.push_back(firstOfLine);
	m_letterCount = m_chars.size();

	// Dimensions du bloc, pour l'ancre
	m_textW = maxPen * size;
	m_textH = (float)(line + 1) * size;

	// L'ancre du bloc vient sur la position : on decale les lettres autour
	// (0,0) = coin haut-gauche sur la position, (0.5,0.5) = bloc centre, etc
	// Les lettres sont centrees sur leur ligne (+/- size/2) : le haut du bloc
	const float anchorDx = -m_anchorX * m_textW;
	const float anchorDy = m_anchorY * m_textH - 0.5f * size;
	if (anchorDx!=0.0f || anchorDy!=0.0f)
	{
		// Decalage dans le repere du groupe (suit la rotation).
		const float shiftX = right.x * anchorDx + up.x * anchorDy;
		const float shiftY = right.y * anchorDx + up.y * anchorDy;
		const float shiftZ = right.z * anchorDx + up.z * anchorDy;
		for (cpu_entity* pEntity : created)
		{
			pEntity->transform.pos.x += shiftX;
			pEntity->transform.pos.y += shiftY;
			pEntity->transform.pos.z += shiftZ;
		}
	}

	// Souligne : bandes derivees des lettres (suivent ancre + groupe)
	if (m_underline)
	{
		const size_t firstStrip = m_entities.size();
		BuildStrips();
		for (size_t i=firstStrip ; i<m_entities.size() ; ++i)
			created.push_back(m_entities[i]);
	}

	return created;
}

// Deplace tout le texte : chaque lettre bouge du meme decalage

void Text3D::SetPosition(float x, float y, float z)
{
	Move(x - m_origin.x, y - m_origin.y, z - m_origin.z);
}

void Text3D::Move(float dx, float dy, float dz)
{
	for (cpu_entity* pEntity : m_entities)
	{
		pEntity->transform.pos.x += dx;
		pEntity->transform.pos.y += dy;
		pEntity->transform.pos.z += dz;
	}
	m_origin.x += dx;
	m_origin.y += dy;
	m_origin.z += dz;
}

// Tourne tout le texte autour de sa position (m_origin)
// Les lettres tournent sur place ET autour du pivot : le texte reste rigide
// La meme matrice sert pour les positions et les orientations

void Text3D::AddYPR(float yaw, float pitch, float roll)
{
	// Comme cpu_transform::AddYPR : autour des axes LOCAUX du groupe
	// (yaw = autour de up, pitch = autour de right, roll = autour de dir)
	XMVECTOR qRot = XMQuaternionIdentity();
	if ( roll!=0.0f )
		qRot = XMQuaternionMultiply(qRot, XMQuaternionRotationAxis(XMLoadFloat3(&m_dir), roll));
	if ( pitch!=0.0f )
		qRot = XMQuaternionMultiply(qRot, XMQuaternionRotationAxis(XMLoadFloat3(&m_right), pitch));
	if ( yaw!=0.0f )
		qRot = XMQuaternionMultiply(qRot, XMQuaternionRotationAxis(XMLoadFloat3(&m_up), yaw));
	qRot = XMQuaternionNormalize(qRot);
	const XMMATRIX rot = XMMatrixRotationQuaternion(qRot);

	// Le groupe a tourne
	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), rot));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), rot));
	XMStoreFloat3(&m_dir, XMVector3TransformNormal(XMLoadFloat3(&m_dir), rot));

	if (m_entities.empty())
		return;
	const XMVECTOR pivot = XMLoadFloat3(&m_origin);

	for (cpu_entity* pEntity : m_entities)
	{
		// Position : rotation autour du pivot
		XMVECTOR offset = XMVectorSubtract(XMLoadFloat3(&pEntity->transform.pos), pivot);
		XMStoreFloat3(&pEntity->transform.pos, XMVectorAdd(XMVector3TransformCoord(offset, rot), pivot));	// Orientation : les 3 axes tournent pareil
		XMStoreFloat3(&pEntity->transform.right, XMVector3TransformNormal(XMLoadFloat3(&pEntity->transform.right), rot));
		XMStoreFloat3(&pEntity->transform.up, XMVector3TransformNormal(XMLoadFloat3(&pEntity->transform.up), rot));
		XMStoreFloat3(&pEntity->transform.dir, XMVector3TransformNormal(XMLoadFloat3(&pEntity->transform.dir), rot));
		pEntity->transform.SetRotationFromAxes();
	}
}

// Change l'ancre : on deplace les lettres (pas la position)
// Le pivot de rotation (m_origin) devient le point d'ancre

void Text3D::SetAnchor(float ax, float ay)
{
	if (ax<0.0f) 
		ax = 0.0f; 
	else if (ax>1.0f) 
		ax = 1.0f;
	if (ay<0.0f) 
		ay = 0.0f; 
	else if (ay>1.0f) 
		ay = 1.0f;

	// Decalage differentiel par rapport a l'ancienne ancre
	const float dx = -(ax - m_anchorX) * m_textW;
	const float dy = (ay - m_anchorY) * m_textH;
	if ((dx!=0.0f || dy!=0.0f) && m_entities.empty()==false)
	{
		for (cpu_entity* pEntity : m_entities)
		{
			pEntity->transform.pos.x += dx;
			pEntity->transform.pos.y += dy;
		}
	}

	m_anchorX = ax;
	m_anchorY = ay;
}

// Styles : memorises + appliques au texte

void Text3D::SetOutline(XMFLOAT3 color, float thicknessPx)
{
	m_outlineColor = color;
	m_outlinePx = (thicknessPx<0.0f) ? 0.0f : thicknessPx;
	TextStyle* pOutlineStyle = CurrentStyle();
	if (pOutlineStyle!=nullptr)
	{
		pOutlineStyle->outlineColor = m_outlineColor;
		pOutlineStyle->outlinePx = (int)(m_outlinePx + 0.5f);
	}
}

void Text3D::SetBold(bool bold)
{
	m_bold = bold;
	TextStyle* pBoldStyle = CurrentStyle();
	if (pBoldStyle!=nullptr)
		pBoldStyle->bold = m_bold;
}

void Text3D::SetItalic(bool italic)
{
	m_italic = italic;
	// Echange les rectangles, les lettres ne bougent pas
	for (size_t i=0 ; i<m_letterCount && i<m_entities.size() && i<m_chars.size() ; ++i)
		m_entities[i]->pMesh = GetGlyphMesh(m_chars[i], m_italic);
}

void Text3D::SetUnderline(bool underline, XMFLOAT3 color)
{
	// Couleur precisee (composantes >= 0) : on la garde; Sinon : couleur du texte
	if (color.x>=0.0f && color.y>=0.0f && color.z>=0.0f)
	{
		m_underlineColor = color;
		m_underlineColorSet = true;
	}
	m_underline = underline;
	if (m_underline)
	{
		m_flatMaterial.color = m_underlineColorSet ? m_underlineColor : m_builtColor;
		if (m_letterCount>0 && m_entities.size()==m_letterCount)
			BuildStrips();
	}
	else
		ClearStrips();
}

// Bandes du souligne : une par ligne, dans le repere de sa 1re lettre
// Comme ca elles suivent le texte, meme apres deplacement/rotation

void Text3D::BuildStrips()
{
	ClearStrips();
	if (m_underline==false || m_letterCount==0)
		return;

	// Couleur choisie, sinon celle du texte
	m_flatMaterial.color = m_underlineColorSet ? m_underlineColor : m_builtColor;

	m_strips.reserve(m_strips.size() + m_lineW.size());
	const float gapCells = 0.10f;	// sous la lettre (unites lettre)
	const float thickCells = 0.08f;	// epaisseur (unites lettre)
	XMFLOAT3 white = CPU_WHITE;
	for (size_t l=0 ; l<m_lineW.size() ; ++l)
	{
		if (m_lineFirst[l]<0 || (size_t)m_lineFirst[l]>=m_letterCount)
			continue;
		if (m_lineW[l]<=0.0f)
			continue;
		cpu_entity* pRef = m_entities[(size_t)m_lineFirst[l]];
		const float yTop = -0.5f - gapCells;
		XMFLOAT3 tl = { 0.0f, yTop, 0.0f };
		XMFLOAT3 tr = { m_lineW[l], yTop, 0.0f };
		XMFLOAT3 br = { m_lineW[l], yTop - thickCells,0.0f };
		XMFLOAT3 bl = { 0.0f, yTop - thickCells, 0.0f };
		m_strips.emplace_back();
		cpu_mesh& strip = m_strips.back();
		strip.AddFace(tl, tr, br, bl, white);	// devant
		strip.AddFace(bl, br, tr, tl, white);	// derriere
		strip.CalculateBoundingVolumes();		// sinon invisible
		cpu_entity* pStrip = cpuEngine.CreateEntity();
		pStrip->pMesh = &strip;
		pStrip->pMaterial = &m_flatMaterial;
		pStrip->transform.pos = pRef->transform.pos;	// meme repere que la lettre
		pStrip->transform.sca = pRef->transform.sca;
		pStrip->transform.dir = pRef->transform.dir;
		pStrip->transform.right = pRef->transform.right;
		pStrip->transform.up = pRef->transform.up;
		pStrip->transform.quat = pRef->transform.quat;
		pStrip->transform.rot = pRef->transform.rot;
		pStrip->transform.ResetFlags();
		m_entities.push_back(pStrip);
	}
}

void Text3D::ClearStrips()
{
	if ( m_letterCount>m_entities.size() )
		return;	// incoherent : on ne touche a rien
	for (size_t i=m_letterCount ; i<m_entities.size() ; ++i)
		cpuEngine.Release(m_entities[i]);
	m_entities.erase(m_entities.begin() + m_letterCount, m_entities.end());
	m_strips.clear();
}

Text3D::TextStyle* Text3D::CurrentStyle()
{
	if (m_styles.empty())
		return nullptr;
	return m_styles.back().get();
}

// Supprime lettres et couleurs

void Text3D::Release()
{
	for (cpu_entity* pEntity : m_entities)
		cpuEngine.Release(pEntity);
	m_entities.clear();
	m_strips.clear();	// les bandes meurent avec leurs lettres
	m_chars.clear();
	m_lineW.clear();
	m_lineFirst.clear();
	m_letterCount = 0;
	m_origin = { 0, 0, 0 };

	for (cpu_material* pMaterial : m_materials)
		delete pMaterial;
	m_materials.clear();
	m_styles.clear();	// apres les materiaux qui les utilisent
}

Text3D::~Text3D()
{
	// Release() doit avoir ete appele avant; On jette juste les couleurs
	for (cpu_material* pMaterial : m_materials)
		delete pMaterial;
	m_materials.clear();
	m_entities.clear();
}

// Orientation du prochain Build (le Build utilise m_right / m_up / m_dir)
void Text3D::SetBaseOrientation(const XMFLOAT3& right, const XMFLOAT3& up, const XMFLOAT3& dir)
{
	m_right = right;
	m_up = up;
	m_dir = dir;
}
