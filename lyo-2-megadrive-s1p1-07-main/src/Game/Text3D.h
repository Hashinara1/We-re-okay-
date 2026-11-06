#pragma once
#include <map>     // glyphes par lettre
#include <memory>   // styles du texte (un par Build) (necessaire pour unique_ptr)
#include <vector>   // lettres / bandes / lignes

// Text3D : affiche du texte en 3D.
// Chaque lettre = un petit rectangle avec une photo de lettre dessus
// Les parties vides de la photo sont jetees (voir le shader)
// Les lettres se voient des 2 cotes
//
//   Une photo = une image en memoire (largeur x hauteur x 4 couleurs :
//   bleu, vert, rouge, transparence); Windows dessine toutes les lettres
//   dedans, les unes a cote des autres : c'est l'atlas
//   Chaque rectangle ne montre qu'un bout de cette grande photo
//   (le bout avec sa lettre); Le reste de la photo est vide/invisible
//
// Exemple :
//   Text3D m_text;
//   m_text.Init();										// au demarrage
//   m_text.Build("SCORE", XMFLOAT3(0, 5, 15), 1.0f);	// texte debout
//   m_text.SetPosition(-4, 0.5f, 10);					// on le deplace
//   m_text.AddYPR(0, XM_PI * 0.5f, 0);					// on le couche sur la table
//   m_text.SetAnchor(0.5f, 0.5f);						// centre sur la position
//   m_text.SetOutline(XMFLOAT3(0, 0, 0), 2.0f);		// contour noir de 2px
//   m_text.SetBold(true);								// gras
//   m_text.Release();									// a la sortie
//
// Un Build() cree une entite par lettre (+ une bande par ligne si souligne)
// Les rectangles sont partages : un par lettre (deux si italique : droit + penche)
//
// Le texte se deplace/tourne comme un groupe : SetPosition / Move / AddYPR
// s'appliquent a toutes les lettres en gardant leur disposition
// SetAnchor choisit le point d'ancrage (0..1, 0..1) : centrage + pivot
// Pour toucher une lettre seule : GetEntities()
//
// Pour un score qui change : reconstruire seulement quand la valeur change
// (Build remplace deja le precedent, pas besoin de Release entre deux)
//   if (score != m_lastScore) {
//       m_lastScore = score;
//       m_score.Build(std::to_string(score).c_str(), pos, size, color);
//   }

class Text3D
{
private:
	// Style d'un texte, lu par le shader (via material->values)
	// Underline et Italic ne sont pas necessaire au shader, donc pas dedans
	struct TextStyle
	{
		XMFLOAT3 outlineColor = { 0.0f, 0.0f, 0.0f };
		int outlinePx = 0;	// epaisseur du contour (pixels atlas), 0 = sans
		bool bold = false;	// gras
	};

public:
	Text3D() = default;
	~Text3D();

	// Prepare la police
	// A appeler une fois au debut
	bool Init(int fontPx = 64, cstr fontName = "Consolas");

	// Cree le texte dans l'orientation du groupe (debout, face -Z, au premier Build),
	// remplace le precedent (pas besoin de Release). Ensuite : SetPosition / Move / AddYPR
	std::vector<cpu_entity*> Build(cstr text, XMFLOAT3 origin = { 0, 0, 0 }, float size = 1.0f, XMFLOAT3 color = CPU_WHITE);

	// Orientation de base du prochain Build (debout, face -Z, par defaut).
	// Pour un texte face camera : passer des axes qui regardent la camera.
	void SetBaseOrientation(const XMFLOAT3& right, const XMFLOAT3& up, const XMFLOAT3& dir);

	// Deplace tout le texte (comme transform.SetPosition)
	void SetPosition(float x, float y, float z);
	// Decale tout le texte
	void Move(float dx, float dy, float dz);
	// Tourne tout le texte autour de sa position (comme transform.AddYPR)
	// Ex : AddYPR(0, PI/2, 0) couche le texte recto vers le haut, lisible depuis +Z
	// (-PI/2 = verso vers le haut, lisible depuis -Z : lettres visibles des 2 cotes)
	void AddYPR(float yaw, float pitch = 0.0f, float roll = 0.0f);

	// Ancre du texte (0..1, 0..1) : ou se trouve la position dans le bloc
	// (0,0) = coin haut-gauche (defaut), (0.5,0.5) = centre,
	// (1,1) = coin bas-droit. Change le centrage ET le pivot de rotation
	// Gardee d'un Build a l'autre (comme les styles et la rotation)
	void SetAnchor(float ax, float ay);
	XMFLOAT2 GetAnchor() const { return { m_anchorX, m_anchorY }; }

	// Styles : immediats, avant ou apres Build (gardes d'un Build a l'autre)
	void SetOutline(XMFLOAT3 color, float thicknessPx = 2.0f); // contour, 0px = sans
	void SetBold(bool bold);		// gras (lettre epaissie)
	void SetItalic(bool italic);	// italique (lettre penchee)
	// Souligne (une bande par ligne); Sans couleur = couleur du texte
	// Ex : SetUnderline() / SetUnderline(false) / SetUnderline(true, XMFLOAT3(1, 0, 0)).
	void SetUnderline(bool underline = true, XMFLOAT3 color = { -1.0f, -1.0f, -1.0f });

	// Position du groupe (= origin du Build, + deplacements)
	XMFLOAT3 GetPosition() const { return m_origin; }
	// Acces aux entites (lettres + bandes du souligne) : reglage fin
	const std::vector<cpu_entity*>& GetEntities() const { return m_entities; }

	// Supprime le texte cree; A appeler a la sortie (Build remplace deja le precedent)
	void Release();

	bool IsReady() const { return m_ready; }

private:
	// La police impose une taille speciale (puissance de 2), voir le .cpp
	bool CreateAtlas(int fontPx, cstr fontName);
	static int NextPow2(int value);

	// Donne le rectangle d'une lettre (cree une fois, reutilise ensuite)
	cpu_mesh* GetGlyphMesh(char c, bool italic);

	// Bandes du souligne, derivees des lettres (suivent ancre + groupe)
	void BuildStrips();
	void ClearStrips();
	// Style du texte (nullptr si aucun) : le modifier = effet immediat
	TextStyle* CurrentStyle();

	// Cree une lettre : rectangle + couleur + position + orientation
	cpu_entity* CreateLetter(char c, cpu_material* pMaterial, XMFLOAT3 pos, float size,
		const XMFLOAT3& right, const XMFLOAT3& up, const XMFLOAT3& dir);

	// Espace apres une lettre
	float GetAdvance(char c) const;

	// Garde seulement les pixels pleins de la lettre, jette le vide
	static void GlyphPixelShader(cpu_ps_io& io);

private:
	cpu_font m_font;									// lettres dessinees par Windows
	cpu_texture m_atlas;								// photo des lettres, taille ajustee
	cpu_material m_glyphMaterial;						// photo + shader, sans couleur
	std::vector<cpu_material*> m_materials;				// une couleur par texte cree
	std::map<int, cpu_mesh> m_glyphs;					// un rectangle par lettre (+ italique)
	std::vector<cpu_entity*> m_entities;				// toutes les lettres creees
	XMFLOAT3 m_origin = { 0, 0, 0 };					// pivot du groupe (deplacement/rotation)
	XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f };			// orientation du groupe (gardee au Build)
	XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 m_dir = { 0.0f, 0.0f, 1.0f };
	std::vector<cpu_mesh> m_strips;						// bandes du souligne
	cpu_material m_flatMaterial;						// bandes unies (sans photo) : texte sauf si precisee
	XMFLOAT3 m_builtColor = { 1, 1, 1 };				// couleur du texte
	std::vector<char> m_chars;							// une lettre par entite
	std::vector<float> m_lineW;							// largeur de chaque ligne
	std::vector<int> m_lineFirst;						// 1re lettre de chaque ligne (-1 si vide)
	size_t m_letterCount = 0;							// lettres dans m_entities
	std::vector<std::unique_ptr<TextStyle>> m_styles;	// un style par texte
	float m_anchorX = 0.0f;								// ancre imposee en x (0..1)
	float m_anchorY = 0.0f;								// ancre imposee en y (0..1)
	float m_textW = 0.0f;								// largeur du bloc (unites monde)
	float m_textH = 0.0f;								// hauteur du bloc (unites monde, lignes * taille)
	XMFLOAT3 m_outlineColor = { 0, 0, 0 };				// couleur du contour
	float m_outlinePx = 0.0f;							// epaisseur du contour (pixels atlas), 0 = sans
	bool m_bold = false;								// gras
	bool m_italic = false;								// italique
	bool m_underline = false;							// souligne
	XMFLOAT3 m_underlineColor = { 1, 1, 1 };			// couleur du souligne (si precisee)
	bool m_underlineColorSet = false;					// false = souligne couleur du texte
	float m_cellWidth = 1.0f;							// largeur d'une lettre (hauteur = 1)
	bool  m_ready = false;								// true quand Init() a reussi
};
