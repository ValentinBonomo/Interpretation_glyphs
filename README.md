# Interpretation Glyphs

Systeme de reconnaissance de glyphes dessines par le joueur dans Unreal Engine 5.7, base sur l'algorithme **$P Point-Cloud Recognizer** (Wobbrock et al. 2012).

## Fonctionnalites

- **Dessin en temps reel** via un widget UMG avec Render Target
- **Reconnaissance $P** : Resample, Scale, Translate, GreedyCloudMatch
- **Extraction automatique** des points depuis une image (Data Asset)
- **Spawn dynamique** d'acteurs ou de meshes 250 unites devant le joueur
- **VFX Niagara** a l'apparition de l'objet reconnu
- **Post-process encre** : effet Sobel sur SceneDepth (style Okami)

## Architecture

```
Source/interpretation_glyph/
  GlyphRecognizer.h/.cpp          -- Algorithme $P complet
  GlyphTemplateAsset.h/.cpp       -- Data Asset (points, image, seuil, spawn, VFX)
  GlyphDrawingWidget.h/.cpp       -- Widget UMG (dessin + reconnaissance)
  GlyphPlayerController.h/.cpp    -- Input toggle (Enhanced Input)
```

## Algorithme $P

Pipeline de reconnaissance :

1. **Resample** : redistribue N points equidistants sur le trace
2. **ScaleToUnitSquare** : normalise dans un carre unitaire
3. **TranslateToOrigin** : centre sur l'origine
4. **GreedyCloudMatch** : comparaison bidirectionnelle avec poids decroissant

Formule de poids : `w = 1 - index / (N - 1)`

Step greedy : `floor(sqrt(N))`

## Data Asset

Chaque glyphe est defini par un `UGlyphTemplateAsset` :

| Champ | Description |
|---|---|
| GlyphName | Nom du glyphe |
| SourceImage | Texture pour extraction auto des points |
| PixelThreshold | Seuil de luminosite pour la detection |
| RawPoints | Points du template (auto ou manuels) |
| NumPoints | Nombre de points apres resample |
| MatchThreshold | Score minimum pour valider la reconnaissance |
| ActorToSpawn | Classe d'acteur a spawn |
| MeshToSpawn | Static Mesh a spawn |
| SpawnVFX | Systeme Niagara a jouer au spawn |

## Post-Process

Effet contour encre base sur la profondeur de scene :

- SceneDepth divise par 1000
- DDX + DDY pour detection de bords (Sobel simplifie)
- Multiplicateur ajustable
- Lerp entre la scene originale et la couleur d'encre

## Prerequis

- Unreal Engine 5.7
- Plugin Niagara active

## Utilisation

1. Creer un `GlyphTemplateAsset` dans le Content Browser
2. Assigner une image source et cliquer sur **Extract Points From Image**
3. Configurer l'acteur/mesh/VFX a spawn
4. Ajouter le template dans le `GlyphDrawingWidget`
5. Configurer le `BP_GlyphPlayerController` avec l'input de toggle
6. Lancer le jeu, appuyer sur la touche configuree, dessiner le glyphe

## Reference

Vatavu, R.-D., Anthony, L. and Wobbrock, J.O. (2012). $P Point-Cloud Recognizer.
