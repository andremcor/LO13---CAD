/*
----------------------------------------------------------------------------------------------------------------------------------
---------- Nom: LO13_correia.cpp
------ Objetif: Créer un programme pour réproduire graphiquement un objet défini par la matrice objet
-- Application: Creation d'un cube et d'un maillage (tigre/tour eiffel/cube) et ses parametres graphiques
------- Auteur: Andre Mormul Correia
--------- Date: 15/06/2023
------ Version: 9.0
----------------------------------------------------------------------------------------------------------------------------------
*/

//==================================================================================================================
// LISTE DES BIBLIOTHEQUES ET DEFINITION DES VARIABLES GLOBALES
//==================================================================================================================

#include <stdlib.h>       // allocation dynamique de mémoire et la manipulation de variables d'environnement
#include "GUI.h"          // le code utilise une bibliothèque ou un module spécifique
#include <math.h>         // utilisé pour les opérations mathématiques
#include <malloc.h>       // allouer et libérer dynamiquement de la mémoire
#include <stdio.h>        // fournit des fonctions d'entrée et de sortie de données

#define ZOOMSEUIL 10.0    // Valeur max/min de zoom possible
#define FZOOM 0.95        // Taux de zoom - fluidité d'agrandissement
#define SROTATION 0.3     // Taux de sensibilite de rotation
#define DIM 3             // Nombre de coordonees par triangle

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// INITIALISATION DES PARAMETRES
//==================================================================================================================

/*
----- Nom: material
---- Type: typedef struct
- Objetif: Il s’agit la d’affecter un matériau au modèle discret afin d’effectuer un ombrage de ce modèle
-- Autres: -
*/
typedef struct material
{
    float ambient[4];        // ka -> Reflection partie ambient
    float diffuse[4];        // kd -> Reflection partie diffuse
    float specular[4];       // kd -> Reflection partie specular
    float shininess;         // ns -> Coefficient matériau caractérisant la diminution de l’intensité autour de la réflexion parfaite
} Material ;

/*
----- Nom: source
---- Type: typedef struct
- Objetif: Pour attribuer les spécifications de la source lumineuse
-- Autres: source est appele par source
*/
typedef struct source
{
    float position[4];       // Coordonees homogenes de la source
    float ambient[4];        // Intensite partie ambient
    float diffuse[4];        // Intensite partie diffuse
    float specular[4];       // Intensite partie specular
} Source;

/*
----- Nom: opengl
---- Type: typedef struct
- Objetif: Definiton de structuree OpenGl afin de definir les variables et
           parametres de vue et graphiques (source, materieaux)
-- Autres: opengl est appele par ogl
*/
typedef struct opengl
{
    int fox, foy, fdx, fdy;                                            // Le positionnement de la fenetre graphique et ses dimensions
    float obsx, obsy, obsz;                                            // Les coordonnes de l'observateur 1
    float focalx, focaly, focalz;                                      // Les coordonnes du point focal (centre du cube)
    float verticalx, verticaly, verticalz;                             // Le vecteur vertical du repere de vue
    float umin, umax, vmin, vmax, dmin, dmax;                          // Le paralepipede de visualisation
    float fu, fv;                                                      // Taux pour ajuster la fenetre graphique
    float fzoom;                                                       // Valeur du zoom
    int flagrotation, flagtranslation;                                 // 1 ou 0 button
    float cx, cy, vx, vy;                                              // Les coordonnees de la souris
    float rotationV, rotationU, translationV, translationU;            // Vecteur pour ratationer l'objet selon les les coordonnees de la souris
    float tg[16];                                                      // Matrice historique des transformations
    float focalu, focalv, OvF;                                         // Distance point focal
    int mode;                                                          // Type de display pour le objet
    float lfg, hfg;                                                    // Dimension de la fenetre
    int ligne, matchoix;                                               // Variable pour le choix du materiaux
    Source src;                                                        // Variable pour la lumiere
    float couleurfond[3];                                              // Valeurs pour la couleur du fond
    float couleurligne[3];                                             // Valeurs pour la couleur de la ligne
    Material steel, cooper, plastic, shadow, *mat;                     // Materiaux
    char *file_name_open;                                              // Dossier (Cube - Tiger - Eiffel)
    char *shrinkonoff;
    int shrinkok;
    float fshrink, fexpand;                                            // Variable pour shrink
    float obsx3d, obsy3d, obsz3d;                                      // Les coordonnes de l'observateur 2
    float shadowx, shadowy, shadowz;                                   // Les coordonnes pour le shadow
}Opengl;
Opengl ogl;

/*
----- Nom: mesh
---- Type: typedef struct
- Objetif: Definiton de structuree mesh afin de definir les variables et des parametres des triangles
-- Autres: Mesh est appele par msh
*/
typedef struct mesh
{
    int number_of_vertices;            // nombre de sommets
    float *vertices;                   // liste des coordonnees des sommets
    float *normal_v;                   // liste des normales aux sommets
    int number_of_triangles;           // nombre de triangles
    int *triangles;                    // liste des sommets des triangles
    float *alpha_t;                    // liste des angles aux sommets
    float *normal_t;                   // liste des normales aux faces triangulaires
    float ccenter[3];                  // coordonnees du centre de la boite englobante
    float cmin[3], cmax[3];            // coordonnees min et max de la boite englobante
    float delta;                       // taille maximale de la boite englobante suivant les axes
    int memory;                        // memoire allouee pour la structure mesh
    int error;                         // arret du programme si error est non nul
    int dirc[5];                       // permutation cyclique des indices
    float *planscarre;                 // liste des coordonnees des plans
}Mesh;
Mesh msh;

/*
----- Nom: InitialiserLibrairieGraphique
---- Type: function
- Objetif: Il est utilisé pour initialiser la bibliothèque GLUT
-- Autres: -
-- Return: Void
*/
void InitialiserLibrairieGraphique(int *argc, char **argv)
{
    glutInit(argc, argv);
}

/*
----- Nom: InitialiserParametresGraphiques
---- Type: function
- Objetif: Quantification des variables et des parametres des triangles
-- Autres: Initialisation de la matrice 4x4 unité (matrice identite)
           Proprietes des materiaux -> http://devernay.free.fr/cours/opengl/materials.html
-- Return: Void
*/
void InitialiserParametresGraphiques()
{
    ogl.fox = 100;                            // Position par rapport a l'ecran de l'ordinateur
    ogl.foy = 100;
    //ogl.fdx = 500;
    //ogl.fdy = 500;

    ogl.lfg = 500;                            // Dimension de la fenetre graphique
    ogl.hfg = 500;

    ogl.obsx = 4;                             // Position observateur principal
    ogl.obsy = 6;
    ogl.obsz = 3;

    ogl.obsx3d = ogl.obsx + 0.05;             // Position observateur 3D
    ogl.obsy3d = ogl.obsy + 0.05;
    ogl.obsz3d = ogl.obsz + 0.05;

    ogl.shadowx = ogl.obsx - 0.2;             // Position observateur ombre
    ogl.shadowy = ogl.obsy + 0.0;
    ogl.shadowz = ogl.obsz + 0.8;

    ogl.focalx = 0.5;                         //Les coordonnes du point focal
    ogl.focaly = 0.5;
    ogl.focalz = 0.5;

    ogl.verticalx = 0;                        // Le vecteur vertical du repere de vue
    ogl.verticaly = 0;
    ogl.verticalz = 1.0;

    ogl.fu = 1.0;                             // Taux pour ajuster la fenetre graphique
    ogl.fv = 1.0;

    ogl.umin = -0.5;                          // Le paralepipede de visualisation, où l'obejet est placé
    ogl.umax = 0.5;
    ogl.vmin = -0.5;
    ogl.vmax = 0.5;
    ogl.dmin = 5.0;
    ogl.dmax = 100.0;

    ogl.fzoom = 1.0;                          // Valeur du zoom

    ogl.flagrotation = 0.0;
    ogl.flagtranslation = 0.0;

    ogl.cx = 0.0;                             // Les coordonnees de la souris
    ogl.cy = 0.0;

    ogl.mode = 1;                             // Type de display pour le objet, initialise en 1
    ogl.matchoix = 0;                         // Type de materiau pour le objet, initialise avec 0

    ogl.ligne = 0;                            // Variable pour le choix du materiaux

    ogl.focalu = 0.0;                         // Distances point focal (observateur et paralepipede)
    ogl.focalv = 0.0;
    ogl.OvF = pow( pow(ogl.focalx - ogl.obsx, 2) + pow(ogl.focaly - ogl.obsy, 2) + pow(ogl.focalz - ogl.obsz, 2), 0.5);

    for( int r = 0; r < 16; r++ ){                   // Matrice unité 4x4 (matrice identite)
            ogl.tg[r] = 0.0;
            if( r == 0 || r == 5 || r == 10 || r == 15 ){
                ogl.tg[r] = 1.0;
            }
    }

    // Couleur space
    ogl.couleurfond[0] = 0.0;
    ogl.couleurfond[1] = 0.0;
    ogl.couleurfond[2] = 0.0;

    // Couleur ligne
    ogl.couleurligne[0] = 1.0;
    ogl.couleurligne[1] = 1.0;
    ogl.couleurligne[2] = 1.0;

    // Coordonees homogenes de la source
    ogl.src.position[0] = 1.0;
    ogl.src.position[1] = 1.0;
    ogl.src.position[2] = 1.0;
    ogl.src.position[3] = 0.0;

    // Source general ambient qui affecte uniformément tous les objets de la scène
    ogl.src.ambient[0] = 1.0;
    ogl.src.ambient[1] = 1.0;
    ogl.src.ambient[2] = 1.0;
    ogl.src.ambient[3] = 1.0;
    // Source general diffuse est la couleur d'un objet lorsqu'il est éclairé par cette source de lumière
    ogl.src.diffuse[0] = 1.0;
    ogl.src.diffuse[1] = 1.0;
    ogl.src.diffuse[2] = 1.0;
    ogl.src.diffuse[3] = 1.0;
    // Source general specular est la couleur qu'acquiert un objet lorsqu'il réfléchit la lumière
    ogl.src.specular[0] = 1.0;
    ogl.src.specular[1] = 1.0;
    ogl.src.specular[2] = 1.0;
    ogl.src.specular[3] = 1.0;

    // Proprietes des materiaux -> http://devernay.free.fr/cours/opengl/materials.html

    // Materiaux couvre
    ogl.cooper.ambient[0] = 0.19125;
    ogl.cooper.ambient[1] = 0.0735;
    ogl.cooper.ambient[2] = 0.0225;
    ogl.cooper.ambient[3] = 1.0;

    ogl.cooper.diffuse[0] = 0.7038;
    ogl.cooper.diffuse[1] = 0.27048;
    ogl.cooper.diffuse[2] = 0.0828;
    ogl.cooper.diffuse[3] = 1.0;

    ogl.cooper.specular[0] = 0.256777;
    ogl.cooper.specular[1] = 0.137622;
    ogl.cooper.specular[2] = 0.086014;
    ogl.cooper.specular[3] = 1.0;

    ogl.cooper.shininess = 0.1;

    // Materiaux plastique
    ogl.plastic.ambient[0] = 0.1;
    ogl.plastic.ambient[1] = 0.25;
    ogl.plastic.ambient[2] = 0.1;
    ogl.plastic.ambient[3] = 1.0;

    ogl.plastic.diffuse[0] = 0.1;
    ogl.plastic.diffuse[1] = 0.35;
    ogl.plastic.diffuse[2] = 0.1;
    ogl.plastic.diffuse[3] = 1.0;

    ogl.plastic.specular[0] = 0.45;
    ogl.plastic.specular[1] = 0.55;
    ogl.plastic.specular[2] = 0.45;
    ogl.plastic.specular[3] = 1.0;

    ogl.plastic.shininess = 0.25;

    // Materiaux acier
    ogl.steel.ambient[0] = 0.25;
    ogl.steel.ambient[1] = 0.25;
    ogl.steel.ambient[2] = 0.25;
    ogl.steel.ambient[3] = 1.0;

    ogl.steel.diffuse[0] = 0.4;
    ogl.steel.diffuse[1] = 0.4;
    ogl.steel.diffuse[2] = 0.4;
    ogl.steel.diffuse[3] = 1.0;

    ogl.steel.specular[0] = 0.774597;
    ogl.steel.specular[1] = 0.774597;
    ogl.steel.specular[2] = 0.774597;
    ogl.steel.specular[3] = 1.0;

    ogl.steel.shininess = 0.6;

    //Materiaux
    ogl.mat = &(ogl.plastic);                  // choix du materiau

    ogl.file_name_open = ("Tiger.mesh");
    ogl.shrinkonoff = ("Trace Filaire");

    ogl.fshrink = 1.0;                         // Variable initiale pour commencer le shrink

    ogl.shrinkok = 0;
}

/*
----- Nom: InitializeMesh
---- Type: function
- Objetif: Initialisation de la struture mesh (elle initialise tous les champs d’une variable de type Mesh)
           Les valeurs dependent du fichier ouvert, ici on les initialise
-- Autres: FUNCTION QUI FAIT L'ATTRIBUITION: READMESH
-- Return: Void
*/
void InitializeMesh()
{
    msh.number_of_vertices = 0.0;
    msh.number_of_triangles = 0.0;
    msh.vertices  = NULL;
    msh.normal_v  = NULL;
    msh.triangles = NULL;
    msh.alpha_t   = NULL;
    msh.normal_t  = NULL;

    for (int j = 0; j < DIM; j++)
    {
        msh.ccenter[j] = 0.0;
        msh.cmin[j] = 0.0;
        msh.cmax[j] = 0.0;
    }

    msh.delta = 0.0;
    msh.memory = 0.0;
    msh.error = 0.0;
    for (int j = 0; j < 2*DIM-1; j++)
        msh.dirc[j] = j % DIM;
}

/*
----- Nom: ReadMesh
---- Type: function
- Objetif: Les instructions utiles pour l’ouverture d’un fichier (fopen)au format mesh et la
           lecture (fscanf) des informations concernant la triangulation sous-jacente
-- Autres: Lecture du fichier au format mesh INRIA Gamma3 -> IL FAIT L'ATTRIBUITION POUR LES VARIABLE MSH
-- Return: Void
*/
void ReadMesh()
{
    FILE *file;
    char file_name[264], keyword[80];
    int i, j, ii, dimension;

    // Ouvre le fichier de maillage en lecture
    if ((file = fopen( ogl.file_name_open , "r")) == NULL)
    {
        printf("file %s not found\n", file_name);
        msh.error = 100;
        return;
    }

    // boucle de lecture du fichier principal
    for (;;)
    {
        fscanf(file, "%s", keyword);
        // Vérifie si c'est la fin du fichier
        if (strcmp(keyword, "EOF") == 0 || strcmp(keyword, "End") == 0 || strcmp(keyword, "end") == 0) // fin du fichier
            break;
        // Vérifier la taille du maillage
        else if (strcmp(keyword, "Dimension") == 0 || strcmp(keyword, "dimension") == 0)               // mot cle dimension
            fscanf(file, "%d", &dimension);
        // Lire les sommets du maillage
        else if (strcmp(keyword, "Vertices") == 0 || strcmp(keyword, "vertices") == 0)                 // mot cle Vertices
        {
            fscanf(file, "%d", &(msh.number_of_vertices));
            // Allouer de la mémoire pour les sommets
            msh.vertices = (float *)malloc((DIM * msh.number_of_vertices) * sizeof(float));
            if (!msh.vertices)
            {
                printf("not enough memory for %d vertices (%ld bytes)\n", msh.number_of_vertices, DIM * msh.number_of_vertices * sizeof(float));
                msh.error = 200;
                return;
            }
            else
                msh.memory += (DIM * msh.number_of_vertices) * sizeof(float);
            // Lit les coordonnées des sommets
            if (dimension == 2)
            {
                for (i = 0; i < msh.number_of_vertices; i++)
                {
                    ii = DIM * i;
                    fscanf(file, "%f %f %d", &(msh.vertices[ii]), &(msh.vertices[ii + 1]), &j);
                    msh.vertices[ii + 2] = 0.0;
                }
            }
            else if (dimension == 3)
            {
                for (i = 0; i < msh.number_of_vertices; i++)
                {
                    ii = DIM * i;
                    fscanf(file, "%f %f %f %d", &(msh.vertices[ii]), &(msh.vertices[ii + 1]), &(msh.vertices[ii + 2]), &j);
                }
            }
        }
        // Lire les triangles du maillage
        else if (strcmp(keyword, "Triangles") == 0 || strcmp(keyword, "triangles") == 0)               // mot cle Triangles
        {
            fscanf(file, "%d", &(msh.number_of_triangles));
            // Allouer de la mémoire pour les triangles
            msh.triangles = (int*)malloc((DIM * msh.number_of_triangles) * sizeof(int));
            if (!msh.triangles)
            {
                printf("not enough memory for %d triangles (%ld bytes)\n", msh.number_of_triangles, DIM * msh.number_of_triangles * sizeof(int));
                msh.error = 200;
                return;
            }
            else
                msh.memory += (DIM * msh.number_of_triangles) * sizeof(int);
            // Lire les indices des sommets des triangles
            for (i = 0; i < msh.number_of_triangles; i++)
            {
                ii = DIM * i;
                fscanf(file, "%d %d %d %d", &(msh.triangles[ii]), &(msh.triangles[ii + 1]), &(msh.triangles[ii + 2]), &j);
                // Définit les indices de sommet pour qu'ils commencent à 0
                for (j = 0; j < DIM; j++)
                    msh.triangles[ii + j]--;
            }
        }
    }
    fclose(file);
    // Affichage a l'ecran des infos sur le maillage
    printf("mesh : %d vertices -- %d triangles (%ld kbytes)\n", msh.number_of_vertices, msh.number_of_triangles, msh.memory / 1024);
}

/*
----- Nom: NormalizeMesh
---- Type: function
- Objetif: Cette procedure effectue une mise à l’echelle des coordonnées des sommets dans le cube unite
-- Autres: (1) Elle utilise une translation et une homothetie pour faire entrer et
           centrer le modele dans le cube unite
           (2) Normalisation des coordonnees des sommets entre 0 et 1 et centrage
-- Return: Void
*/
void NormalizeMesh()
{
    int i, ii, j;

    // Initialise les valeurs minimum et maximum des bornes (cmin et cmax) avec les valeurs du premier sommet
    for (j = 0; j < DIM; j++)
    {
        msh.cmin[j] = msh.vertices[j];
        msh.cmax[j] = msh.vertices[j];
    }

    // Boucle sur les sommets restants et met à jour les valeurs minimale et maximale des limites
    for (i = 1; i < msh.number_of_vertices; i++)
    {
        ii = DIM * i;
        for (j = 0; j < DIM; j++)
        {
            if (msh.cmin[j] > msh.vertices[ii + j])
                msh.cmin[j] = msh.vertices[ii + j];
            if (msh.cmax[j] < msh.vertices[ii + j])
                msh.cmax[j] = msh.vertices[ii + j];
        }
    }

    // Calcule le centre du maillage (ccenter) comme la moyenne des limites
    for (j = 0; j < DIM; j++)
        msh.ccenter[j] = (float)(0.5 * (msh.cmin[j] + msh.cmax[j]));
    msh.delta = msh.cmax[0] - msh.cmin[0];
    // Calcule la valeur delta comme la différence entre le maximum et le minimum dans chaque dimension
    for (j = 1; j < DIM; j++)
        if (msh.delta < msh.cmax[j] - msh.cmin[j])
            msh.delta = msh.cmax[j] - msh.cmin[j];
    // Normalise les coordonnées des sommets à une échelle de 0 à 1
    for (i = 0; i < msh.number_of_vertices; i++)
    {
        ii = DIM * i;
        for (j = 0; j < DIM; j++)
            msh.vertices[ii + j] = (float)(0.5 + (msh.vertices[ii + j] - msh.ccenter[j]) / msh.delta); // met les coordonnées a l'échelle
    }
}

/*
----- Nom: InitialiserGeometrie
---- Type: function
- Objetif:
           (1) Creer toutes les coordonnes dans une list de chaquee sommet (8 sommets chaqu'un avec 3 coordonnes)
           (2) Creer matrice 4x4 identite -> tg
           (3) Creer 2 triangules pour chaque face et leur atribuir pour chaque sommet ses coordonnes
-- Autres: -
-- Return: Void
*/
void InitialiserGeometrie()
{
    msh.number_of_triangles = 12;
    msh.number_of_vertices = 8;
    msh.vertices = (float*)malloc((DIM*msh.number_of_vertices)*sizeof(float));
    msh.triangles = (int*)malloc((DIM*msh.number_of_triangles)*sizeof(int));

    for( int e = 0; e < 24; e++ ){
            msh.vertices[e] = 0.0;
            if(e == 3 || e == 6 || e == 7 || e == 10 || e == 14 || e == 15 || e == 17 || e == 18 || e == 19 || e == 20 || e == 22 || e == 23 ){
                msh.vertices[e] = 1.0;
            }
    }

    // Triangles de la FACE
    msh.triangles[0]  = 5;
    msh.triangles[1]  = 1;
    msh.triangles[2]  = 2;
    msh.triangles[3]  = 6;
    msh.triangles[4]  = 5;
    msh.triangles[5]  = 2;
    // Triangles du SOMMET
    msh.triangles[6]  = 4;
    msh.triangles[7]  = 5;
    msh.triangles[8]  = 6;
    msh.triangles[9]  = 4;
    msh.triangles[10] = 6;
    msh.triangles[11] = 7;
    // Triangles de la DROITE
    msh.triangles[12] = 7;
    msh.triangles[13] = 6;
    msh.triangles[14] = 2;
    msh.triangles[15] = 7;
    msh.triangles[16] = 2;
    msh.triangles[17] = 3;
    // Triangles de la GAUCHE
    msh.triangles[18] = 1;
    msh.triangles[19] = 5;
    msh.triangles[20] = 0;
    msh.triangles[21] = 5;
    msh.triangles[22] = 4;
    msh.triangles[23] = 0;
    // Triangles du BAS
    msh.triangles[24] = 0;
    msh.triangles[25] = 2;
    msh.triangles[26] = 1;
    msh.triangles[27] = 0;
    msh.triangles[28] = 3;
    msh.triangles[29] = 2;
    // Triangles d'ARRIERE
    msh.triangles[30] = 7;
    msh.triangles[31] = 3;
    msh.triangles[32] = 4;
    msh.triangles[33] = 4;
    msh.triangles[34] = 3;
    msh.triangles[35] = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// INITIALISATION DE LA FENETRE
//==================================================================================================================

/*
----- Nom: BasePourMatriceModelVue
---- Type: function
- Objetif: Base pour MatriceModelVue(), MatriceModelVue3D() et MatriceModelVueShadow()
-- Autres: -
-- Return: Void
*/
void BasePourMatriceModelVue(){
    glMatrixMode(GL_MODELVIEW);                                 // definit la matrice de transformation actuelle en tant que modèle et matrice de vue
    glLoadIdentity();                                           // réinitialise la matrice actuelle à la matrice d'identité
    if ( ogl.flagrotation == 1 ){
            Rotation();
    } else if ( ogl.flagtranslation == 1 ){
            Translation();
    }
    glMultMatrixf(ogl.tg);                                      // Appliquer toutes les transformations geometriques antecedentes
    glGetFloatv(GL_MODELVIEW_MATRIX, ogl.tg);                   // Mettre a jour l’historique des transformations geometriques
}

/*
----- Nom: MatriceModelVue
---- Type: function
- Objetif: Pour appliquer une transformation geométrique donnée, on doit non seulement composer
           cette transformation mais aussi toutes les transformations antecédentes pour la continuite
           des transformations. La function definit tous les parametres utilises dans l'ecran
-- Autres: -
-- Return: Void
*/
void MatriceModelVue(){
    BasePourMatriceModelVue();
    gluLookAt(ogl.obsx, ogl.obsy, ogl.obsz,
              ogl.focalx, ogl.focaly, ogl.focalz,
              ogl.verticalx, ogl.verticaly, ogl.verticalz);     // Appliquer le changement de repere avec les coordenees d'observateur, centre et hauteur
}

/*
----- Nom: MatriceModelVue3d
---- Type: function
- Objetif: Pour appliquer une transformation geométrique donnée, on doit non seulement composer
           cette transformation mais aussi toutes les transformations antecédentes pour la continuite
           des transformations. La function definit tous les parametres utilises dans l'ecran
-- Autres: -
-- Return: Void
*/
void MatriceModelVue3d(){
    BasePourMatriceModelVue();
    gluLookAt(ogl.obsx3d, ogl.obsy3d, ogl.obsz3d,
              ogl.focalx, ogl.focaly, ogl.focalz,
              ogl.verticalx, ogl.verticaly, ogl.verticalz);     // Appliquer le changement de repere avec les coordenees d'observateur, centre et hauteur
}

/*
----- Nom: MatriceModelVueShadow
---- Type: function
- Objetif: Pour appliquer une transformation geométrique donnée, on doit non seulement composer
           cette transformation mais aussi toutes les transformations antecédentes pour la continuite
           des transformations. La function definit tous les parametres utilises dans l'ecran
-- Autres: -
-- Return: Void
*/
void MatriceModelVueShadow(){
    BasePourMatriceModelVue();
    gluLookAt(ogl.shadowx, ogl.shadowy, ogl.shadowz,
              ogl.focalx, ogl.focaly, ogl.focalz,
              ogl.verticalx, ogl.verticaly, ogl.verticalz);     // Appliquer le changement de repere avec les coordenees d'observateur, centre et hauteur
}

/*
----- Nom: MatriceProjection
---- Type: function
- Objetif: Creer le paralellepipede autour de tout (champs de vision) -> Tpers
-- Autres: -
-- Return: Void
*/
void MatriceProjection()
{
    glMatrixMode(GL_PROJECTION);              // définit la matrice de transformation actuelle comme matrice de projection
    glLoadIdentity();                         // réinitialise la matrice actuelle à la matrice d'identité
    glFrustum(ogl.fzoom*ogl.fu*ogl.umin,
              ogl.fzoom*ogl.fu*ogl.umax,
              ogl.fzoom*ogl.fv*ogl.vmin,
              ogl.fzoom*ogl.fv*ogl.vmax,
              ogl.dmin,
              ogl.dmax);                      // Creer le paralellepipede autour de tout (champs de vision) volume d'affichage
}

/*
----- Nom: EffacerEcran
---- Type: function
- Objetif: Effacer tous qu'il y a sur la fennetre graphique
-- Autres: -
-- Return: Void
*/
void EffacerEcran()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);   // efface le contenu des tampons de couleur et de profondeur
}

/*
----- Nom: InitialiserEnvironnementGraphique
---- Type: function
- Objetif: Spécifier des valeurs claires pour les tampons de couleur et après définir la couleur actuelle
-- Autres: -
-- Return: Void
*/
void InitialiserEnvironnementGraphique()
{
    glClearColor(ogl.couleurfond[0], ogl.couleurfond[1], ogl.couleurfond[2], 0.0);    // Definition de la couleur du fond
    glColor3f(ogl.couleurligne[0], ogl.couleurligne[1], ogl.couleurligne[2]);         // Definition de la couleur courante de la ligne
    MatriceProjection();                                                              // Champs de vision ou volume d'affichage
}

/*
----- Nom: CreationFenetreGraphique
---- Type: function
- Objetif: Creer l'ecran et sa taille, le coordonne de l'ecran
-- Autres: -
-- Return: Void
*/
void CreationFenetreGraphique()
{
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);    // définit le mode d'affichage de la fenêtre OpenGL
    glutInitWindowSize(ogl.lfg, ogl.hfg);                    // space ocupe par la fenetre
    glutInitWindowPosition(ogl.fox, ogl.foy);                // position du point focal
    glutCreateWindow("Projet_LO13_CORREIA_Andre");           // creation dela fenetre graphique et le nome d'affichage
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// TRACE DES OBJETS
//==================================================================================================================

/*
----- Nom: TracerObjet
---- Type: function
- Objetif: Pour chaque ensemble de point on crée un triangle (chaque 'i' est le sommet d'un triangle
-- Autres: -
-- Return: Void
*/
void TracerObjet()
{
    // début de la définition des triangles pour le rendu
    glBegin(GL_TRIANGLES);
    // Boucle à travers les triangles du maillage
    for(int i = 0; i < msh.number_of_triangles; i++)
    {
        int i1 = 3*(i);
        for ( int j = 0 ; j <= 2; j++)
        {
            int s = msh.triangles[i1+j];
            int s1 = DIM*(s);
            // Définit les coordonnées du sommet actuel et l'ajoute au tampon de rendu
            glVertex3f(msh.vertices[s1], msh.vertices[s1+1], msh.vertices[s1+2]);
        }
    }
    // Terminer la définition des triangles
    glEnd();
}

/*
----- Nom: TracerObjetDegrade
---- Type: function
- Objetif: Pour chaque ensemble de point on cree un triangle degradee (chaque 'i' est le sommet d'un triangle)
-- Autres: -
-- Return: Void
*/
void TracerObjetDegrade()
{
    glBegin(GL_TRIANGLES);

    int i, j, i1, s;
    float A, B, C, A1, B1,C1;
    double Aw[3], Bw[3], Cw[3], G[3];                               // Point A, B et C initial et P
    float A_ligne[3], B_ligne[3], C_ligne[3], GA[3],GB[3],GC[3];

    for(i=0;i<msh.number_of_triangles;i++)
    {
        glColor3f((float)(msh.number_of_triangles-i)/msh.number_of_triangles,0.0,(float)i/msh.number_of_triangles);

        i1= 3*i; //triângulo i
        s=3*msh.triangles[i1+0]; // s é a coordenada zero do primeiro sommet do triângulo i

        Aw[0] = msh.vertices[s];
        Aw[1] = msh.vertices[s + 1];
        Aw[2] = msh.vertices[s + 2];

        s=3*msh.triangles[i1+1]; //coord do 2o sommet do triangulo i
        Bw[0] = msh.vertices[s];
        Bw[1] = msh.vertices[s + 1];
        Bw[2] = msh.vertices[s+ 2];

        s=3*msh.triangles[i1+2]; //coord do 3o sommet do triângulo i
        Cw[0] = msh.vertices[s];
        Cw[1] = msh.vertices[s + 1];
        Cw[2] = msh.vertices[s + 2];

        G[0] = Aw[0]/3.0 + Bw[0]/3.0 + Cw[0]/3.0;                          //Projection de changement
        G[1] = Aw[1]/3.0 + Bw[1]/3.0 + Cw[1]/3.0;
        G[2] = Aw[2]/3.0 + Bw[2]/3.0 + Cw[2]/3.0;

        int s1;
        for(j=0;j<=2;j++)
        {
            s=msh.triangles[i1+j];
            s1=3*(s);

            glNormal3f(msh.normal_v[s1], msh.normal_v[s1+1], msh.normal_v[s1+2]);
            A1 = msh.vertices[s1]*ogl.fshrink +(1.0 - ogl.fshrink)*G[0];
            B1 = msh.vertices[s1+1]*ogl.fshrink +(1.0 - ogl.fshrink)*G[1];
            C1 = msh.vertices[s1+2]*ogl.fshrink +(1.0 - ogl.fshrink)*G[2];
            glVertex3f(A1 ,B1, C1);
        }
    }
    glEnd();
}

/*
----- Nom: TraceTriangle
---- Type: function
- Objetif: Pour chaque ensemble de point on cree un triangle unique (chaque 'i' est le sommet d'un triangle)
-- Autres: -
-- Return: Void
*/
void TraceTriangle()
{
    int i, j, s;
    // début de la définition des triangles pour le rendu
    glBegin(GL_TRIANGLES);
    // Boucle à travers les triangles du maillage                                                           // delimiter les sommet
    for (i=0 ; i < msh.number_of_triangles ; i++)
    {
        // groupe des trois sommets du triangles i pour former un triangle
        for (j=0 ; j<3 ; j++)
        {
        // s est le numéro du sommet j du triangle i
        s = msh.triangles[3*i+j] ;
        glVertex3f(msh.vertices[3*s], msh.vertices[3*s+1], msh.vertices[3*s+2]) ;  //Specifier un vertice
        }
    }
    // delimiter les sommets
    glEnd();
}

/*
----- Nom: TracerCarre
---- Type: function
- Objetif: Pour chaque ensemble de point on crée un carre pour avoir les plans de projection OXYZ
-- Autres: -
-- Return: Void
*/
void TracerCarre()
{
    glBegin(GL_TRIANGLES);
    //for(int i = 0; i < 36; i++)
    //{
        //int i1 = 3*(i);
        //for ( int j = 0 ; j <= 2; j++)
        //{
           // int s = msh.triangles[i1+j];
            //int s1 = DIM*(s);
            //glVertex3f(msh->planscarre[0], 0, 1);
            //glVertex3f(0, 0, 0);
            //glVertex3f(0, 0, 1);
       //}
    //}
    glEnd();
}


/*
----- Nom: ObjetOmbrageConstant
---- Type: fonction
- Objetif: L’ombrage du modele discret peut etre vu comme choisir une couleur pour chaque
           element (triangle). Le choix de la couleur est effectue au moment ou l’on balaye
           les elements pour les dessiner.
-- Autres: La procedure comprend l’activation de la source et l’affectation du materiau
-- Return: Void
*/
void ObjetOmbrageConstant()
{
    int i, j, s;

    ActivationZBuffer();
    ActivationSource();
    AffectationMateriau();
    glShadeModel(GL_FLAT);     // mode ombrage constant
    TracerShrink();
    DesactivationSource();
    DesactivationZBuffer();
}

/*
----- Nom: ObjetOmbragePhong
---- Type: fonction
- Objetif: C’est une procedure similaire a l’ombrage constant sauf que le mode est different (GL_SMOOTH)
-- Autres: La procedure comprend l’activation de la source et l’affectation du materiau
-- Return: Void
*/
void ObjetOmbragePhong()
{
    int i, j, s;
    ActivationZBuffer();
    ActivationSource();
    AffectationMateriau();
    glShadeModel(GL_SMOOTH);      // Mode ombrage de Phong
    TracerShrink();
    DesactivationSource();
    DesactivationZBuffer();
}

/*
----- Nom: ActivationZBuffer
---- Type: function
- Objetif: Le traitement des parties cachees permet d’eliminer les parties de
           l’objet qui ne sont pas visibles de l’observateur
-- Autres: L’activation du z-buffer
-- Return: Void
*/
void ActivationZBuffer()
{
    glEnable(GL_DEPTH_TEST);                    // activer le test de profondeur
}

/*
----- Nom: DesactivationZBuffer
---- Type: function
- Objetif: Le traitement des parties cachees permet d’eliminer les parties de
           l’objet qui ne sont pas visibles de l’observateur
-- Autres: La desactivation du z-buffer
-- Return: Void
*/
void DesactivationZBuffer()
{
    glDisable(GL_DEPTH_TEST);                   // desactiver le test de profondeur
}

/*
----- Nom: DecalageAvantActivation
---- Type: function
- Objetif: Decalage avant pour le contour (Tracer en mode remplissage,
           avancer l’objet d’un petit pas vers l’observateur)
-- Autres: Eviter polygones superposés
-- Return: Void
*/
void DecalageAvantActivation(){
    glPolygonOffset(1.0, 1.0);                     // Décalage de polygone
    glEnable(GL_POLYGON_OFFSET_FILL);              // Decalage avant pour le contour
}
void DecalageAvantActivationLine(){
    glPolygonOffset(1.0, 1.0);                     // Décalage de polygone
    glEnable(GL_POLYGON_OFFSET_LINE);              // Decalage avant pour le contour
}

/*
----- Nom: DecalageAvantDesactivation
---- Type: function
- Objetif: Apres tracer en mode contour, remettre l’objet a sa position d’origine
-- Autres: -
-- Return: Void
*/
void DecalageAvantDesactivation()
{
    glDisable(GL_POLYGON_OFFSET_FILL);
}

/*
----- Nom: Shrink
---- Type: Ensemble de function
- Objetif: Pour detecter facilement visuellement certains defauts dans un maillage,
           il peut etre utile de reduire la taille des elements
-- Autres: -
-- Return: Void
*/
//----------------------------------------------------------------------------------------------------------------------------------

/*
----- Nom: Shrink
---- Type: Ensemble de function
- Objetif: Pour detecter facilement visuellement certains defauts dans un maillage,
           il peut etre utile de reduire la taille des elements
-- Autres: -
-- Return: Void
*/
void TracerShrink()
{
    glBegin(GL_TRIANGLES);                                          // début de la définition des triangles pour le rendu

    int j, i1, s;
    float A, B, C, A1, B1,C1;
    double Aw[3], Bw[3], Cw[3], G[3];                               // Point A, B et C initial et P
    float A_ligne[3], B_ligne[3], C_ligne[3], GA[3],GB[3],GC[3];

    for(int i = 0; i < msh.number_of_triangles; i++)
    {
        i1= 3*i; //triângulo i
        s=3*msh.triangles[i1+0];                                    // s é a coordenada zero do primeiro sommet do triângulo i
        Aw[0] = msh.vertices[s];                                    //X Premier triangle
        Aw[1] = msh.vertices[s + 1];                                //Y Premier triangle
        Aw[2] = msh.vertices[s + 2];                                //Z Premier triangle

        s=3*msh.triangles[i1+1];                                     //coord do 2o sommet do triangulo i
        Bw[0] = msh.vertices[s];                                    //X Deuxieme triangle
        Bw[1] = msh.vertices[s + 1];                                //Y Deuxieme triangle
        Bw[2] = msh.vertices[s + 2];                                //Z Deuxieme triangle

        s=3*msh.triangles[i1+2];                                    //coord do 3o sommet do triângulo i
        Cw[0] = msh.vertices[s];                                    //X Troisieme triangle
        Cw[1] = msh.vertices[s + 1];                                //Y Troisieme triangle
        Cw[2] = msh.vertices[s + 2];                                //Z Troisieme triangle

        G[0] = Aw[0]/3.0 + Bw[0]/3.0 + Cw[0]/3.0;                   //Projection de changement pour le baricentre
        G[1] = Aw[1]/3.0 + Bw[1]/3.0 + Cw[1]/3.0;                   //Projection de changement pour le baricentre
        G[2] = Aw[2]/3.0 + Bw[2]/3.0 + Cw[2]/3.0;                   //Projection de changement pour le baricentre

        int s1;
        for( int j = 0; j <= 2; j++)
        {
            s = msh.triangles[i1+j];
            s1 = 3 * (s);

            glNormal3f(msh.normal_v[s1], msh.normal_v[s1+1], msh.normal_v[s1+2]);
            A1 = msh.vertices[s1] * ogl.fshrink + ( 1.0 - ogl.fshrink) * G[0];          // decalage du point
            B1 = msh.vertices[s1+1] * ogl.fshrink + ( 1.0 - ogl.fshrink) * G[1];        // decalage du point
            C1 = msh.vertices[s1+2] * ogl.fshrink + ( 1.0 - ogl.fshrink) * G[2];        // decalage du point

            glVertex3f(A1 ,B1, C1);                                                     // Les coordonnees du nouveau triangle
        }
    }
    glEnd();                                                                            // Terminer la définition des triangles
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// LES MODES DES TRACE
//==================================================================================================================


/*
----- Nom: TracerFilaire
---- Type: function
- Objetif: Structure trace standard
-- Autres: Mode 1
-- Return: Void
*/
void TracerFilaire()
{
    textEcran(ogl.file_name_open);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);                                // Mode de trace en filaire
    glColor3f(ogl.couleurligne[0], ogl.couleurligne[1], ogl.couleurligne[2]); // Definition de la couleur courante de la ligne
    TracerShrink();                                                                 // Type de trace
    glutSwapBuffers();                                                        // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: TracerRemplissageFilaireUnie
---- Type: function
- Objetif: Structure trace et ramprissage une couleur
-- Autres: Mode 2
-- Return: Void
*/
void TracerRemplissageFilaireUnie()
{
    textEcran(ogl.file_name_open);
    ActivationZBuffer();                                                       // activer le test de profondeur
    DecalageAvantActivation();                                                 // Décalage de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                                 // rendu des polygones remplis
    glColor3f(0.3, 0.8, 0.5);                                                  // Couleur objet
    TracerShrink();                                                                  // Type de trace
    DecalageAvantDesactivation();
    glColor3f(ogl.couleurligne[0], ogl.couleurligne[1], ogl.couleurligne[2]);  // Definition de la couleur courante de la ligne
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);                                 // Mode de trace en filaire
    TracerShrink();                                                                  // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                                         // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: TracerRemplissageDegradeFilaireUnie
---- Type: function
- Objetif: Structure trace et ramprissage de couleur degradee
-- Autres: Mode 3
-- Return: Void
*/
void TracerRemplissageDegradeFilaireUnie()
{
    textEcran(ogl.file_name_open);
    ActivationZBuffer();                                                       // activer le test de profondeur
    DecalageAvantActivation();                                                 // Décalage de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                                 // rendu des polygones remplis
    TracerObjetDegrade();                                                      // Type de trace
    DecalageAvantDesactivation();
    glColor3f(ogl.couleurligne[0], ogl.couleurligne[1], ogl.couleurligne[2]);  // Definition de la couleur courante de la ligne
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);                                  // Mode de trace en filaire
    TracerShrink();                                                             // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                                         // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: TracerRemplissagePartiesCachesFilaireUnie
---- Type: function
- Objetif: Structure trace et ramprissage de couleur neutre
-- Autres: Mode 4
-- Return: Void
*/
void TracerRemplissagePartiesCachesFilaireUnie()
{
    textEcran(ogl.file_name_open);
    ActivationZBuffer();                                                       // activer le test de profondeur
    DecalageAvantActivation();                                                 // Décalage de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                                 // rendu des polygones remplis
    glColor3f(ogl.couleurfond[0], ogl.couleurfond[1], ogl.couleurfond[2]);     // Definition de la couleur du fond
    TracerShrink();                                                                  // Type de trace
    DecalageAvantDesactivation();
    glColor3f(ogl.couleurligne[0], ogl.couleurligne[1], ogl.couleurligne[2]);  // Definition de la couleur courante de la ligne
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);                                 // Mode de trace en filaire
    TracerShrink();                                                                  // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                                         // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: OmbrageConstantPartiesCachees
---- Type: function
- Objetif: Ombrage constant sur les elements avec le traitement des parties cachees
-- Autres: Mode 5
-- Return: Void
*/
void OmbrageConstantPartiesCachees()
{
    ActivationSource();                                // Activation de la source lumineuse
    textEcran(ogl.file_name_open);
    ActivationZBuffer();                               // activer le test de profondeur
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);         // rendu des polygones remplis
    ObjetOmbrageConstant();                            // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                 // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: OmbrageConstantUniquement
---- Type: function
- Objetif: Ombrage constant uniquement sur les aretes avec le traitement des parties cachees
-- Autres: Mode 6
-- Return: Void
*/
void OmbrageConstantUniquement()
{
    ActivationSource();                                                     // Activation de la source lumineuse
    textEcran(ogl.file_name_open);
    ActivationZBuffer();                                                    // activer le test de profondeur
    DecalageAvantActivation();                                              // Décalage de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                              // rendu des polygones remplis
    glColor3f(ogl.couleurfond[0], ogl.couleurfond[1], ogl.couleurfond[2]);  // Definition de la couleur du fond
    TracerShrink();                                                        // Type de trace
    DecalageAvantDesactivation();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);                              // Mode de trace en filaire
    ObjetOmbrageConstant();                                                 // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                                      // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: OmbrageConstantCouleurUnie
---- Type: function
- Objetif: Ombrage constant sur les elements et couleur unie
           pour les aretes avec le traitement des parties cachees
-- Autres: Mode 7
-- Return: Void
*/
void OmbrageConstantCouleurUnie()
{
    ActivationSource();                                                        // Activation de la source lumineuse
    textEcran(ogl.file_name_open);
    DecalageAvantActivation();                                                 // Décalage de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                                 // rendu des polygones remplis
    ObjetOmbrageConstant();                                                    // Type de trace
    DecalageAvantDesactivation();
    ActivationZBuffer();                                                       // activer le test de profondeur
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);                                 // Mode de trace en filaire
    glColor3f(ogl.couleurligne[0], ogl.couleurligne[1], ogl.couleurligne[2]);  // Definition de la couleur courante de la ligne
    TracerShrink();                                                           // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                                         // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: OmbragePhongPartiesCachees
---- Type: function
- Objetif: Ombrage de Phong sur les elements avec le traitement des parties cachees
-- Autres: Mode 8
-- Return: Void
*/
void OmbragePhongPartiesCachees()
{
    ActivationSource();                                // Activation de la source lumineuse
    textEcran(ogl.file_name_open);
    ActivationZBuffer();                               // activer le test de profondeur
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);         // rendu des polygones remplis
    ObjetOmbragePhong();                               // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                 // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: OmbragePhongUniquement
---- Type: function
- Objetif: Ombrage de Phong uniquement sur les arêtes avec le traitement des parties cachees
-- Autres: Mode 9
-- Return: Void
*/
void OmbragePhongUniquement()
{
    ActivationSource();                                                     // Activation de la source lumineuse
    textEcran(ogl.file_name_open);
    ActivationZBuffer();                                                    // activer le test de profondeur
    DecalageAvantActivation();                                              // Décalage de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                              // rendu des polygones remplis
    glColor3f(ogl.couleurfond[0], ogl.couleurfond[1], ogl.couleurfond[2]);  // Definition de la couleur du fond
    TraceTriangle();                                                        // Type de trace
    DecalageAvantDesactivation();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);                              // Mode de trace en filaire
    ObjetOmbragePhong();                                                    // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                                      // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: OmbragePhongCouleurUnie
---- Type: function
- Objetif: Ombrage de Phong sur les elements et couleur unie
           pour les arêtes avec le traitement des parties cachees
-- Autres: Mode 10
-- Return: Void
*/
void OmbragePhongCouleurUnie()
{
    ActivationSource();                                                        // Activation de la source lumineuse
    textEcran(ogl.file_name_open);
    DecalageAvantActivation();                                                 // Décalage de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                                 // rendu des polygones remplis
    ObjetOmbragePhong();                                                       // Type de trace
    DecalageAvantDesactivation();
    ActivationZBuffer();                                                       // activer le test de profondeur
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);                                 // Mode de trace en filaire
    glColor3f(ogl.couleurligne[0], ogl.couleurligne[1], ogl.couleurligne[2]);  // Definition de la couleur courante de la ligne
    TracerShrink();                                                           // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                                         // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: Projection3d
---- Type: function
- Objetif: Structure trace standard avec 2 projection pour avoir une image 3D
-- Autres: Mode 11
-- Return: Void
*/
void Projection3d()
{
    textEcran(ogl.file_name_open);
    ActivationZBuffer();                                                       // activer le test de profondeur
    DecalageAvantActivationLine();                                             // Décalage de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);                                 // Mode de trace en filaire
    glColor3f(1.0, 0.0, 0.0);                                                  // Definition de la couleur rouge
    MatriceModelVue();                                                         // placer les coordenees d'observateur
    TracerShrink();                                                             // Type de trace
    glColor3f(0.2, 0.6, 1.0);                                                  // Definition de la couleur bleu
    MatriceModelVue3d();                                                       // placer les coordenees d'observateur 2
    TracerShrink();                                                             // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                                         // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: Shadow
---- Type: function
- Objetif: Structure trace standard avec 2 projection pour avoir avec l'ombre
-- Autres: Mode 12
-- Return: Void
*/
void Shadow()
{
    ActivationSource();                                                        // Activation de la source lumineuse
    textEcran(ogl.file_name_open);
    ActivationZBuffer();                                                       // activer le test de profondeur
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                                 // rendu des polygones remplis
    MatriceModelVue();                                                         // placer les coordenees d'observateur
    ObjetOmbragePhong();                                                       // Type de trace
    glEnable(GL_DEPTH_TEST);                                                   // activer le test de profondeur
    DecalageAvantActivation();                                                 // Décalage de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                                 // rendu des polygones remplis
    glColor3f(0.4, 0.4, 0.4);                                                  // Definition de la couleur ombre
    MatriceModelVueShadow();                                                   // placer les coordenees d'observateur ombre
    TracerObjet();                                                             // Type de trace
    DesactivationZBuffer();
    glutSwapBuffers();                                                         // L’envoie du buffer graphique sur l’ecran
}

/*
----- Nom: PlansCarre
---- Type: function
- Objetif: Structure trace standard avec 3 projection dans les 3 plans droits
-- Autres: Mode 13
-- Return: Void
*/
void PlansCarres()
{
    textEcran(ogl.file_name_open);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);                                 // Mode de trace en filaire
    glColor3f(ogl.couleurligne[0], ogl.couleurligne[1], ogl.couleurligne[2]);  // Definition de la couleur courante de la ligne
    TracerCarre();                                                             // Type de trace
    glutSwapBuffers();                                                         // L’envoie du buffer graphique sur l’ecran
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// INITIALISATION DU TEXTE
//==================================================================================================================

/*
----- Nom: output
---- Type: function
- Objetif: A pour but de renvoie le text ecrit sur le programme, affichage de text
-- Autres: -
-- Return: Void
*/
void output(GLfloat x, GLfloat y, char *format,...)
{
      va_list args;
      char buffer[200], *p;
      va_start(args, format);
      vsprintf(buffer, format, args);
      va_end(args);
      glPushMatrix();
      glTranslatef(x, y, 0);
      for (p = buffer; *p; p++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
      glPopMatrix();
}

/*
----- Nom: textEcran
---- Type: function
- Objetif: Creation de na fenetre text et ses atributs
-- Autres: -
-- Return: Void
*/
void textEcran(file_nome){

    int i = 0;

    GLfloat position[] = {0.0, 0.0, 1.5, 1.0};

    EffacerEcran();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(0.0, 0.0, -5.0);

    glPushMatrix();
    glRotated(0.0, 1.0, 0.0, 0.0);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glTranslated(0.0, 0.0, 1.5);
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);                               // Couleur des letres
    glPopMatrix();

    glPopMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    float telau = ogl.fu*ogl.umin*6000;
    float telav = ogl.fv*ogl.umin*6000;
    gluOrtho2D(telau, -telau, telav, -telav);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    output(telau + 100, - telav - 200, "0 - 9 -> Modes");
    output(telau + 100, - telav - 350, "M or m -> Materiaux");
    output(telau + 100, - telav - 500, "I or i -> Init.");
    output(telau + 100, - telav - 650, "Z or z -> Zoom In");
    output(telau + 100, - telav - 800, "X or x -> Zoom Out");
    output(telau + 100, - telav - 950, "D or d -> 3D");
    output(telau + 100, - telav - 1100, "C or c -> Shadow");
    output(telau + 100, - telav - 1250, "P or p -> Plans Carres");
    output(telau + 100, - telav - 1400, "S or s -> Shrink +");
    output(telau + 100, - telav - 1550, "A or a -> Shrink -");
    output(telau + 100, - telav - 1700, "Left Button -> Rotat.");
    output(telau + 100, - telav - 1850, "Right Button -> Trans.");
    output(telau + 100, - telav - 2000, "Middle Button -> Menu");
    output(telau + 100, - telav - 2150, "Q or q -> Quit");
    output(telau + 100, - telav - 3000, file_nome);
    switch(ogl.mode){
        case 1:
            output(telau + 100, telav + 100, "Trace Filaire");
            ogl.shrinkonoff = "Trace Filaire";
            ogl.matchoix = 0;
            break;
        case 2:
            output(telau + 100, telav + 100, "Trace Rempl. Filaire Unie");
            ogl.shrinkonoff = "Trace Rempl. Filaire Unie";
            ogl.matchoix = 0;
            break;
        case 3:
            output(telau + 100, telav + 100, "Trace Rempl. Deg. Filaire Unie");
            ogl.shrinkonoff = "Trace Rempl. Deg. Filaire Unie";
            ogl.matchoix = 0;
            break;
        case 4:
            output(telau + 100, telav + 100, "Trace Rempl. Parties Caches Filaire Unie");
            ogl.shrinkonoff = "Trace Rempl. Parties Caches Filaire Unie";
            ogl.matchoix = 0;
            break;
        case 5:
            output(telau + 100, telav + 100, "Ombrage Const. Parties Cachees");
            ogl.shrinkonoff = "Ombrage Const. Parties Cachees";
            ogl.matchoix = 1;
            break;
        case 6:
            output(telau + 100, telav + 100, "Ombrage Const. Uniq.");
            ogl.shrinkonoff = "Ombrage Const. Uniq.";
            ogl.matchoix = 1;
            break;
        case 7:
            output(telau + 100, telav + 100, "Ombrage Const. Couleur Unie");
            ogl.shrinkonoff = "Ombrage Const. Couleur Unie";
            ogl.matchoix = 1;
            break;
        case 8:
            output(telau + 100, telav + 100, "Ombrage Phong Parties Cachees");
            ogl.shrinkonoff = "Ombrage Phong Parties Cachees";
            ogl.matchoix = 1;
            break;
        case 9:
            output(telau + 100, telav + 100, "Ombrage Phong Uniq.");
            ogl.shrinkonoff = "Ombrage Phong Uniq.";
            ogl.matchoix = 1;
            break;
        case 10:
            output(telau + 100, telav + 100, "Ombrage Phong Couleur Unie");
            ogl.shrinkonoff = "Ombrage Phong Couleur Unie";
            ogl.matchoix = 1;
            break;
        case 11:
            output(telau + 100, telav + 100, "Projection 3D");
            ogl.shrinkonoff = "Projection 3D";
            ogl.matchoix = 0;
            break;
        case 12:
            output(telau + 100, telav + 100, "Shadow");
            ogl.shrinkonoff = "Shadow";
            ogl.matchoix = 1;
            break;
        case 13:
            output(telau + 100, telav + 100, "Plans Carres");
            ogl.shrinkonoff = "Plans Carres";
            ogl.matchoix = 1;
            break;
    }

    switch(ogl.shrinkok){
        case 14:
            output(telau + 100, telav + 400, "Shrink -");
            output(telau + 100, telav + 100, ogl.shrinkonoff);
            break;
        case 15:
            output(telau + 100, telav + 400, "Shrink +");
            output(telau + 100, telav + 100, ogl.shrinkonoff);
            break;
        case 0:
            output(telau + 100, telav + 400, "Shrink NULL");
            output(telau + 100, telav + 100, ogl.shrinkonoff);
            break;
    }

    switch(ogl.matchoix){
        case 0:
            output(telau + 100, telav + 250, "Materiaux: ----");
            break;
        case 1:
            switch(ogl.ligne){
                case 0:
                    output(telau + 100, telav + 250, "Materiaux: Plastique");
                    break;
                case 1:
                    output(telau + 100, telav + 250, "Materiaux: Couper");
                    break;
                case 2:
                    output(telau + 100, telav + 250, "Materiaux: Steel");
                    break;
            }
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// DIPLAY
//==================================================================================================================


/*
----- Nom: Display
---- Type: function
- Objetif: Creer l'ecran, vider tous la memoire pour initialiser une nouvelle fenetre plein
-- Autres: -
-- Return: Void
*/
void Display()
{
    EffacerEcran();                // Effacer l’ecran
    MatriceModelVue();             // Definir l’instance GL_MODELVIEW via une procedure utilisateur
    glLineWidth(1.0);              // epaisseur de la ligne
    switch(ogl.mode){
        case 1:
            TracerFilaire();
            break;
        case 2:
            TracerRemplissageFilaireUnie();
            break;
        case 3:
            TracerRemplissageDegradeFilaireUnie();
            break;
        case 4:
            TracerRemplissagePartiesCachesFilaireUnie();
            break;
        case 5:
            OmbrageConstantPartiesCachees();
            break;
        case 6:
            OmbrageConstantUniquement();
            break;
        case 7:
            OmbrageConstantCouleurUnie();
            break;
        case 8:
            OmbragePhongPartiesCachees();
            break;
        case 9:
            OmbragePhongUniquement();
            break;
        case 10:
            OmbragePhongCouleurUnie();
            break;
        case 11:
            Projection3d();
            break;
        case 12:
            Shadow();
            break;
        case 13:
            PlansCarres();
            break;
    }
}

/*
----- Nom: initialisation
---- Type: function
- Objetif: Retourne a position initiale.
           Initialiser la vue a la vue d’origine tout en conservant le mode (annulation
           des transformations geometriques et annulation du « zoom »).
-- Autres: -
-- Return: Void
*/
void initialisation()
{
    ogl.fzoom = 1.0;

    ogl.umin = -0.5;
    ogl.umax = 0.5;

    ogl.vmin = -0.5;
    ogl.vmax = 0.5;

    ogl.dmin = 5.0;
    ogl.dmax = 100.0;

    ogl.tg[0]  = 1.0;
    ogl.tg[1]  = 0.0;
    ogl.tg[2]  = 0.0;
    ogl.tg[3]  = 0.0;
    ogl.tg[4]  = 0.0;
    ogl.tg[5]  = 1.0;
    ogl.tg[6]  = 0.0;
    ogl.tg[7]  = 0.0;
    ogl.tg[8]  = 0.0;
    ogl.tg[9]  = 0.0;
    ogl.tg[10] = 1.0;
    ogl.tg[11] = 0.0;
    ogl.tg[12] = 0.0;
    ogl.tg[13] = 0.0;
    ogl.tg[14] = 0.0;
    ogl.tg[15] = 1.0;

    ogl.fshrink = 1.0;

    MatriceProjection();            // volume d'affichage (parallelepipede) - Creer le paralellepipede autour de tout
    glutPostRedisplay();            // demande une mise à jour de la fenêtre
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// EVENEMENTS
//==================================================================================================================

/*
----- Nom: Reshape
---- Type: function
- Objetif: Le role de reshape est de conserver l’isotropie du systeme, le cube doit rester un
           cube et non pas un parallelepipède allonge
-- Autres: Parametres d'entrees w (width) et h (hight)
-- Return: Void
*/
void Reshape(int lfg, int hfg){
    if ( lfg > hfg ){
        ogl.fu = (float) lfg/hfg;       // Définit le facteur d'échelle horizontale pour le format de la fenêtre
        ogl.fv = 1.0;                   // Définit le facteur d'échelle verticale à 1,0
    } else {
        ogl.fv = (float) hfg/lfg;       // Définit le facteur d'échelle verticale pour le format de la fenêtre
        ogl.fu = 1.0;                   // Définit le facteur d'échelle horizontale à 1,0
    }
    MatriceProjection();                //volume d'affichage (parallelepipede)
    glViewport(0, 0, lfg, hfg);         // Définit la fenêtre d'affichage OpenGL pour qu'elle corresponde à la taille de la fenêtre
}

/*
----- Nom: Keyboard
---- Type: function
- Objetif: Le role de reshape est des que l’on appuie sur une touche du clavier, deux procedures sont à définir
-- Autres: Parametres d'entrees c (type caractère non sige (unsigned char) qui permet d’identifier la
           touche du clavier), cy et cx (coordonnées de la position de la souris)
-- Return: Void
*/
void Keyboard(unsigned char c, int cx, int cy){

    if( c == 'q' || c == 'Q' ){
        exit(0);                               // Le code suivant permet de quitter le programme si on appuie sur ‘q’ ou ‘Q’
    } else if ( c == 'z' || c == 'Z' ){
        ogl.fzoom *= FZOOM;
        if( ogl.fzoom > ZOOMSEUIL ){
                ogl.fzoom *= 1.0/FZOOM;
        }
        if( ogl.fzoom < ZOOMSEUIL ){
                MatriceProjection();           //volume d'affichage (parallelepipede)
        }
    } else if ( c == 'x' || c == 'X' ){
        ogl.fzoom *= 1.0/FZOOM;
        if( ogl.fzoom > ZOOMSEUIL ){
                ogl.fzoom *= FZOOM;
        }
        if( ogl.fzoom < ZOOMSEUIL ){
                MatriceProjection();           //volume d'affichage (parallelepipede)
        }
    } else if ( c == '1' ){
        ogl.mode = 1;
    } else if ( c == '2' ){
        ogl.mode = 2;
    } else if ( c == '3' ){
        ogl.mode = 3;
    } else if ( c == '4' ){
        ogl.mode = 4;
    } else if ( c == '5' ){
        ogl.mode = 5;
    } else if ( c == '6' ){
        ogl.mode = 6;
    } else if ( c == '7' ){
        ogl.mode = 7;
    } else if ( c == '8' ){
        ogl.mode = 8;
    } else if ( c == '9' ){
        ogl.mode = 9;
    } else if ( c == '0' ){
        ogl.mode = 10;
    } else if ( c == 'm' || c == 'M' ){
        switch(ogl.ligne){
        case 0:
            ogl.mat = &(ogl.cooper);                   // choix du materiau
            ogl.ligne = 1;                             // prochain choix du materiau
            break;
        case 1:
            ogl.mat = &(ogl.steel);                    // choix du materiau
            ogl.ligne = 2;                             // prochain choix du materiau
            break;
        case 2:
            ogl.mat = &(ogl.plastic);                  // choix du materiau
            ogl.ligne = 0;                             // prochain choix du materiau
            break;
        }
    } else if (c == 'i' || c == 'I') {
        initialisation();
        ogl.shrinkok = 0;
    }
    else if (c == 'd' || c == 'D')
    {
        ogl.mode = 11;
    }
    else if (c == 'c' || c == 'C')
    {
        ogl.mode = 12;
    }
    else if (c == 'p' || c == 'P')
    {
        ogl.mode = 13;
    }
    else if (c == 'a' || c == 'A')
    {
        ogl.fshrink -= 0.01;
        if (ogl.fshrink <= 0.0)
        {
            ogl.fshrink = 0.0;
        }
        glutPostRedisplay();
        ogl.shrinkok = 14;
    }
    else if (c == 's' || c == 'S')
    {
        ogl.fshrink += 0.01;
        if (ogl.fshrink >= 1.0)
        {
            ogl.fshrink = 1.0;
        }
        glutPostRedisplay();
        ogl.shrinkok = 15;
    }
    glutPostRedisplay();                                                  // Refaire le display
}

/*
----- Nom: Mouse
---- Type: evenement
- Objetif: Cet evenement est initie des que l’on appuie sur un bouton de la souris
-- Autres: (b) -> Le bouton appuier
           (e) -> On/Off bouton
-- Return: Void
*/
void Mouse(int b, int e, int x, int y){
    if( b == GLUT_LEFT_BUTTON )           // GLUT_LEFT_BUTTON ou GLUT_MIDDLE_BUTTON ou GLUT_RIGHT_BUTTON
    {
        if( e == GLUT_DOWN )              // GLUT_UP et GLUT_DOWN
        {
            ogl.flagrotation = 1.0;
            ogl.cx = x;                   // Les coordonnées en pixel de la souris dans la fenetre graphique (Stocker la position de la souris)
            ogl.cy = y;                   // Les coordonnées en pixel de la souris dans la fenetre graphique (Stocker la position de la souris)
        } else if( e == GLUT_UP ){
            ogl.flagrotation = 0.0;
        }
    } else if( b == GLUT_RIGHT_BUTTON )   // GLUT_LEFT_BUTTON ou GLUT_MIDDLE_BUTTON ou GLUT_RIGHT_BUTTON
    {
        if( e == GLUT_DOWN )              // GLUT_UP et GLUT_DOWN
        {
            ogl.flagtranslation = 1.0;
            ogl.cx = x;                   // Les coordonnées en pixel de la souris dans la fenetre graphique (Stocker la position de la souris)
            ogl.cy = y;                   // Les coordonnées en pixel de la souris dans la fenetre graphique (Stocker la position de la souris)
        } else if( e == GLUT_UP ){
            ogl.flagtranslation = 0.0;
        }
    }
}

/*
----- Nom: Motion
---- Type: evenement
- Objetif: Cet evenement est initie dès que l’on bouge la souris (en appuyant ou pas sur un bouton)
-- Autres: -
-- Return: Void
*/
void Motion(int x, int y){

    float vx, vy, dx, dy;                                                     // Declaration du vecteur de déplacement de la souris

    vx = x - ogl.cx;                                                          // Calcule du vecteur déplacement
    vy = y - ogl.cy;                                                          // Calcule du vecteur déplacement

    ogl.cx = x;                                                               // Mettre a jour la position initiale de la souris
    ogl.cy = y;                                                               // Mettre a jour la position initiale de la souris

    if( ogl.flagrotation == 1.0 ){
        ogl.rotationU = SROTATION * vx;                                       // Les angles sont calculés en transformant le vecteur de
        ogl.rotationV = SROTATION * vy;                                       // deplacement de la souris en angles en radian
        glutPostRedisplay();                                                  // Refaire le display (redessiner)
    }

    if( ogl.flagtranslation == 1.0 ){
        dx = vx *( ogl.fzoom * ogl.fu * (ogl.umax - ogl.umin)) / ogl.lfg;     // Calcule du vecteur déplacement
        dy = - vy *( ogl.fzoom * ogl.fv * (ogl.vmax - ogl.vmin)) / ogl.hfg;   // Calcule du vecteur déplacement
        ogl.translationV = ogl.OvF * dx / ogl.dmin;                           // Les distances sont calculés en transformant le vecteur du
        ogl.translationU = ogl.OvF * dy / ogl.dmin;                           // deplacement de la souris
        glutPostRedisplay();                                                  // Refaire le display
    }
}

/*
----- Nom: Rotation
---- Type: evenement
- Objetif: Un deplacement oblique sera une composition de ces deux deplacements de base et correspond
           a une rotation composee de deux rotations de base
-- Autres: -
-- Return: Void
*/
void Rotation(){
    glTranslatef(0.0, 0.0, -ogl.OvF);                   // Translater F a sa position d’origine dans le repère de vue
    glRotatef(ogl.rotationV, 1.0, 0.0, 0.0);            // Rotation autour de l’axe X
    glRotatef(ogl.rotationU, 0.0, 1.0, 0.0);            // Rotation autour de l’axe Y
    glTranslatef(0.0, 0.0, ogl.OvF);                    // Translater F sur Ov dans le repere de vue
}

/*
----- Nom: Translation
---- Type: evenement
- Objetif: Un deplacement oblique sera une composition de ces deux deplacements de base et correspond
           a une translation en Z, Y et X
-- Autres: -
-- Return: Void
*/
void Translation(){
    glTranslatef(ogl.translationV, ogl.translationU, 0.0);           // Rotation autour de l’axe V et U (translation dans le plan de vue)
    ogl.focalu += ogl.translationU;
    ogl.focalv += ogl.translationV;
}

/*
----- Nom: Evenement
---- Type: Ensemble de function
- Objetif: Initialiser les evenements crees
-- Autres: -
-- Return: Void
*/
void Evenement(){
    glutDisplayFunc(Display);                                       // mise à jour de la fenêtre
    glutReshapeFunc(Reshape);                                       // redimensionnement de la fenêtre
    glutKeyboardFunc(Keyboard);                                     // appelé lorsqu'une touche du clavier est faite
    glutMouseFunc(Mouse);                                           // un événement de souris se produit
    glutMotionFunc(Motion);                                         // événement de mouvement de la souris avec un bouton enfoncé
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// SOURCE
//==================================================================================================================

/*
----- Nom: ActivationSource
---- Type: fonction
- Objetif: La procédure ActivationSource() doit comprendre les
           activations et les spécifications (Activation de la source lumineuse)
-- Autres: -
-- Return: Void
*/
void ActivationSource()
{
    glEnable(GL_LIGHTING);                                     //Active le calcul d'intensité
    glEnable(GL_LIGHT0);                                       //Activation de la source
    //Affecte correctement la position de la source imposé
    glMatrixMode(GL_MODELVIEW);                                // définit la matrice courante comme matrice de modélisation et de visualisation
    glPushMatrix();                                            // copier la matrice actuelle et la pousser sur la pile de matrices
    glLoadIdentity();                                          // matrice courante comme matrice d'identité
    glLightfv(GL_LIGHT0, GL_POSITION, ogl.src.position);       // définit les paramètres d'une source lumineuse
    glPopMatrix();                                             // supprime la matrice supérieure de la pile de matrices
    //Affecte
    glLightfv(GL_LIGHT0, GL_AMBIENT, ogl.src.ambient);         // définit les paramètres d'une source lumineuse
    glLightfv(GL_LIGHT0, GL_DIFFUSE, ogl.src.diffuse);         // définit les paramètres d'une source lumineuse
    glLightfv(GL_LIGHT0, GL_SPECULAR, ogl.src.specular);       // définit les paramètres d'une source lumineuse
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);           // définit les paramètres du modèle d'éclairage global
}

/*
----- Nom: DesactivationSource
---- Type: fonction
- Objetif: La procédure DesactivationSource()doit comprendre les désactivations
-- Autres: -
-- Return: Void
*/
void DesactivationSource()
{
    glDisable(GL_LIGHTING);                                     //desactive le calcul d'intensité
    glDisable(GL_LIGHT0);                                       //desactivation de la source
}

/*
----- Nom: AffectationMateriau
---- Type: fonction
- Objetif: Pour affecter un matériau donné au modèle discret (Affectation du Materiau)
-- Autres: -
-- Return: Void
*/
void AffectationMateriau()
{
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (ogl.mat)->ambient);           // définit les paramètres de matériau pour une face donnée de l'objet rendu
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (ogl.mat)->diffuse);           // définit les paramètres de matériau pour une face donnée de l'objet rendu
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (ogl.mat)->specular);         // définit les paramètres de matériau pour une face donnée de l'objet rendu
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (ogl.mat)->shininess*128.0);  // définit les paramètres du modèle d'éclairage global du materiau
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// PROCEDURE POUR LA LECTURE ET LA NORMALISATION DU MAILLAGE ET LE CALCUL DES NORMALES
//==================================================================================================================

double DotProduct(float *vect1, float *vect2)
{
    int i;
    double d;
    d = 0.0;
    for (i=0; i<DIM; i++)
        d += vect1[i] * vect2[i];
    return(d);
}

double NormSquare(float *vect)
{
    return (DotProduct(vect, vect));
}

double Norm(float *vect)
{
    return (sqrt(NormSquare(vect)));
}

void CrossProduct(Mesh *msh, float *vect1, float *vect2, float *result)
{
    int i;
    double norme;

    for (i=0; i<DIM; i++)
        result[i] = vect1[msh->dirc[i+1]]*vect2[msh->dirc[i+2]] - vect1[msh->dirc[i+2]]*vect2[msh->dirc[i+1]];
        norme = Norm(result); // normalisation du produit vectoriel
        if (norme != 0.0)
            {
                norme = 1.0 / norme;
                for (i = 0 ; i < DIM ; i++)
                    result[i] = (float)(norme * result[i]);
            }
}

double Angle(float *point1, float *point2, float *point3) // calcul l'angle entre les vecteurs (point2,point1) et (point2,point3)
{
    double angle, d;
    float v1[3], v2[3];
    int i;

    for (i = 0; i < DIM; i++)
    {
        v1[i] = point1[i] - point2[i];
        v2[i] = point3[i] - point2[i];
    }
    d = DotProduct(v1, v2);
    if (d > 1.0)
        d = 1.0;
    else if (d < -1.0)
        d = -1.0;
        angle = acos(d / sqrt( NormSquare(v1) * NormSquare(v2)));
    return (angle);
}

void SetTriangleNormals(Mesh *msh)
{
    int i, ii, j; // declaration des variables
    float *v0, *v1, *v2, u[3], v[3], n[3];
    double norme;

    msh->normal_t = (float *)malloc((DIM * msh->number_of_triangles) * sizeof(float)); // allocation de la memoire pour les normales aux triangles
    if (!msh->normal_t)                                                                // gestion de l'erreur d'allocation mémoire
    {
        printf("not enough memory for %d normales aux triangles (%ld bytes)\n", msh->number_of_triangles, DIM * msh->number_of_triangles * sizeof(float));
        msh->error = 200;
        return;
    }
    else
        msh->memory += (DIM * msh->number_of_triangles) * sizeof(float);
        // fin de l'allocation de la memoire pour les normales aux triangles
        msh->alpha_t = (float *)malloc((DIM * msh->number_of_triangles) * sizeof(float)); // allocation de la memoire pour les angles des triangles
    if (!msh->alpha_t)                                                               // estion de l'erreur d'allocation mémoire
    {
        printf("not enough memory for %d angles aux triangles (%ld bytes)\n", msh->number_of_triangles, DIM * msh->number_of_triangles * sizeof(float));
        msh->error = 200;
        return;
    }
    else
        msh->memory += (DIM * msh->number_of_triangles) * sizeof(float);
        // fin de l'allocation de la memoire pour les angles des triangles
    for (i = 0; i < msh->number_of_triangles; i++)           // boucle sur les triangles
    {
        ii = DIM * i;
        v0 = &(msh->vertices[DIM * msh->triangles[ii]]);     // premiere coordonnee du premier point du triangle ii
        v1 = &(msh->vertices[DIM * msh->triangles[ii + 1]]); // premiere coordonnee du deuxieme point du triangle ii
        v2 = &(msh->vertices[DIM * msh->triangles[ii + 2]]); // premiere coordonnee du troisieme point du triangle ii
        for (j = 0; j < DIM; j++)                            // calcul des vecteurs du triangles
        {
            u[j] = v1[j] - v0[j];
            v[j] = v2[j] - v0[j];
        }
        CrossProduct(msh, u, v, n);                               // effectue le produit vectoriel de u et v, le résultat est le tableau n
        norme = NormSquare(n);                               // effectue la norme au carre de n (n est un tableau de DIM floats)
        if (norme != 0)
        {
            norme = 1.0 / sqrt(norme);
            msh->normal_t[ii] = (float)(norme * n[0]);
            msh->normal_t[ii + 1] = (float)(norme * n[1]);
            msh->normal_t[ii + 2] = (float)(norme * n[2]);
        }
        else
        {
            msh->normal_t[ii] = n[0];
            msh->normal_t[ii + 1] = n[1];
            msh->normal_t[ii + 2] = n[2];
        }
        // calcul des angles associés àÂ chaque sommet de chaque triangle
        msh->alpha_t[ii] = (float)Angle(v2, v0, v1);        // calcul du premier angle du triangle
        msh->alpha_t[ii + 1] = (float)Angle(v0, v1, v2);    // calcul du deuxieme angle du triangle
        msh->alpha_t[ii + 2] = (float)Angle(v1, v2, v0);    // calcul du troisieme angle du triangle
    }
}

void SetVertexNormals(Mesh *msh)
{
    int i, ii, j, k;
    double norme;

    msh->normal_v = (float *)malloc((DIM * msh->number_of_vertices) * sizeof(float)); // allocation de la memoire des normales aux sommets
    if (!msh->normal_v)                                                               // gestion de l'erreur d'allocation mémoire
    {
        printf("not enough memory for %d normales aux sommets (%ld bytes)\n",
        msh->number_of_vertices, DIM * msh->number_of_vertices * sizeof(float));
        msh->error = 200;
        return;
    }
    else
        msh->memory += (DIM * msh->number_of_vertices) * sizeof(float);
    // fin de l'allocation de la memoire des normales aux sommets
    for (i = 0; i < msh->number_of_vertices; i++)     // début du calcul des normales aux sommets (initialistation a 0)
    {
        ii = DIM * i;
        msh->normal_v[ii] = 0.0;
        msh->normal_v[ii + 1] = 0.0;
        msh->normal_v[ii + 2] = 0.0;
    }
    if (msh->number_of_triangles != 0)                 // test si il y a des triangles
    {
        for (i = 0; i < msh->number_of_triangles; i++) // calcul des normales aux sommets des triangles
        {
            ii = DIM * i;
            for (j = 0; j < DIM; j++)                                                                                // boucle sur les sommets
                for (k = 0; k < DIM; k++)                                                                            // boucle sur les coordonnes
                    msh->normal_v[DIM * msh->triangles[ii + j] + k] += msh->alpha_t[ii + j] * msh->normal_t[ii + k]; // incrémentation pour chaque sommet
        }
    }
    for (i = 0; i < msh->number_of_vertices; i++)        // normalisations des normales
    {
        ii = DIM * i;
        norme = NormSquare(&(msh->normal_v[ii]));
        if (norme != 0)
        {
            norme = 1.0 / sqrt(norme);
            for (j = 0; j < DIM; j++)
                msh->normal_v[ii + j] = (float)(norme * msh->normal_v[ii + j]);
        }
        else
            for (j = 0; j < DIM; j++)
                msh->normal_v[ii + j] = 0.0;
    }
}

void SetNormals(Mesh *msh)
{
    if (msh->number_of_triangles != 0) // test si il y a des triangles
    {
        SetTriangleNormals(msh);
        SetVertexNormals(msh);
    }
}

/*
----- Nom: ModelDiscret
---- Type: Ensemble de function
- Objetif: Initialiser les evenements crees pour les mode des couleurs,des materiaux et des types
-- Autres: -
-- Return: Void
*/
void ModelDiscret(Mesh *msh, char **argv)
{
    InitializeMesh(msh);       // Initialisation de la struture mesh
    ReadMesh(msh);             // Lire la struture mesh
    NormalizeMesh(msh);        // normalizer la struture mesh
    SetNormals(msh);           // Calcul des normales
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// MENU
//==================================================================================================================

/*
----- Nom: MenuModes
---- Type: Ensemble de function
- Objetif: Pour obtenir la valeur d'OpenGL et exécute l'action appropriee selon les materieaux desires
-- Autres: -
-- Return: Void
*/
void Menu(int value){

    switch(value){
        case 10:
            initialisation();
            break;
        case 0:
            exit(0);
        case 1:
            ogl.mat = &(ogl.plastic);
            ogl.ligne = 0;
            break;
        case 2:
            ogl.mat = &(ogl.cooper);
            ogl.ligne = 1;
            break;
        case 3:
            ogl.mat = &(ogl.steel);
            ogl.ligne = 2;
            break;
    }
    glutPostRedisplay();
}

/*
----- Nom: MenuModes
---- Type: Ensemble de function
- Objetif: Pour obtenir la valeur d'OpenGL et exécute l'action appropriee selon le mode desire
-- Autres: -
-- Return: Void
*/
void MenuModes(int value){
    switch(value){
        case 1:
            ogl.mode = 1;
            break;
        case 2:
            ogl.mode = 2;
            break;
        case 3:
            ogl.mode = 3;
            break;
        case 4:
            ogl.mode = 4;
            break;
        case 5:
            ogl.mode = 5;
            break;
        case 6:
            ogl.mode = 6;
            break;
        case 7:
            ogl.mode = 7;
            break;
        case 8:
            ogl.mode = 8;
            break;
        case 9:
            ogl.mode = 9;
            break;
        case 0:
            ogl.mode = 10;
            break;
        case 12:
            ogl.mode = 12;
            break;
        case 13:
            ogl.mode = 13;
            break;
    }
    glutPostRedisplay();
}

/*
----- Nom: MenuSup
---- Type: Ensemble de function
- Objetif: Pour obtenir la valeur d'OpenGL et exécute l'action appropriee selon le mode desire
-- Autres: -
-- Return: Void
*/
void MenuSup(int value){
    switch(value){
        case 1:
            ogl.mode = 11;
            break;
        case 2:
            ogl.mode = 12;
            break;
        case 3:
            ogl.mode = 13;
            break;
    }
    glutPostRedisplay();
}

/*
----- Nom: menuInit
---- Type: Ensemble de function
- Objetif: Pour acceder plus facilement à toutes les options precedentes
           on peut creer un menu utilisable avec la souris
-- Autres: La fonction qui enregistre un menu de fonction qui gérera le menu.
-- Return: Void
*/
void menuInit(){

    int sub2 = glutCreateMenu(MenuModes);
    // ajouter une entrée à un menu
	glutAddMenuEntry("( 1 ) Trace Filaire", 1);
	glutAddMenuEntry("( 2 ) Trace Rempl. Filaire Unie", 2);
	glutAddMenuEntry("( 3 ) Trace Rempl. Deg. Filaire Unie", 3);
	glutAddMenuEntry("( 4 ) Trace Rempl. Parties Caches Filaire Unie", 4);
	glutAddMenuEntry("( 5 ) Ombrage Const. Parties Cachees", 5);
	glutAddMenuEntry("( 6 ) Ombrage Const. Uniq.", 6);
	glutAddMenuEntry("( 7 ) Ombrage Const. Couleur Unie", 7);
	glutAddMenuEntry("( 8 ) Ombrage Phong Parties Cachees", 8);
	glutAddMenuEntry("( 9 ) Ombrage Phong Uniq.", 9);
	glutAddMenuEntry("( 0 ) Ombrage Phong Couleur Unie", 0);
	// créer un nouveau menu
	glutCreateMenu(MenuModes);

    int sub1 = glutCreateMenu(Menu);
    // ajouter une entrée à un menu
	glutAddMenuEntry("Platic (M or m)", 1);
	glutAddMenuEntry("Couper (M or m)", 2);
	glutAddMenuEntry("Steel  (M or m)", 3);

    int sub3 = glutCreateMenu(MenuSup);
    // ajouter une entrée à un menu
	glutAddMenuEntry("(D-c) Projection 3D", 1);
	glutAddMenuEntry("(C-c) Shadow", 2);
	glutAddMenuEntry("(P-p) Plans Carres", 3);
	// créer un nouveau menu
	glutCreateMenu(MenuSup);

	// créer un nouveau menu
	glutCreateMenu(Menu);

	// ajoute un sous-menu à un menu existant
    glutAddSubMenu("Materials", sub1);
	glutAddSubMenu("Modes (0 - 9)", sub2);
	glutAddSubMenu("Supplementary", sub3);

	// ajouter une entrée à un menu
	glutAddMenuEntry("Quit   (Q or q)", 0);
	glutAddMenuEntry("Init   (I or i)", 10);

	// Attach the menu to the middle button
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

//----------------------------------------------------------------------------------------------------------------------------------

//==================================================================================================================
// MAIN
//==================================================================================================================

/*
----- Nom: main
---- Type: function principale
- Objetif: Initialiser le code
-- Autres: -
-- Return: Le programme
*/
int main(int argc, char **argv)
{
    InitialiserLibrairieGraphique(&argc, argv);
    InitialiserParametresGraphiques();
    ModelDiscret(&msh, argv);
    CreationFenetreGraphique();
    InitialiserEnvironnementGraphique();
    Evenement();
    menuInit();
    glutMainLoop();                                 // lance la boucle principale du programme graphique
    return(0);
}
