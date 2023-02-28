//*** Solutionnaire version 2, sans les //[ //] au bon endroit car le code est assez différent du code fourni.
#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures_solutionnaire_2.hpp"      // Structures de données pour la collection de films en mémoire.

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "memory"
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include "cppitertools/range.hpp"
#include "gsl/span"
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).
using namespace iter;
using namespace gsl;

#pragma endregion//}

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{

UInt8 lireUint8(istream& fichier)
{
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier)
{
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}


void ListeFilms::changeDimension(int nouvelleCapacite)
{
	Film** nouvelleListe = new Film*[nouvelleCapacite];
	
	if (elements != nullptr) { 
		nElements = min(nouvelleCapacite, nElements);
		for (int i : range(nElements))
			nouvelleListe[i] = elements[i];
		delete[] elements;
	}
	
	elements = nouvelleListe;
	capacite = nouvelleCapacite;
}

void ListeFilms::ajouterFilm(Film* film)
{
	if (nElements == capacite)
		changeDimension(max(1, capacite * 2));
	elements[nElements++] = film;
}

span<Film*> ListeFilms::enSpan() const { return span(elements, nElements); }

void ListeFilms::enleverFilm(const Film* film)
{
	for (Film*& element : enSpan()) {  
		if (element == film) {
			if (nElements > 1)
				element = elements[nElements - 1];
			nElements--;
			return;
		}
	}
}

span<shared_ptr<Acteur>> spanListeActeurs(const ListeActeurs& liste) {
	return span<shared_ptr<Acteur>>(liste.elements.get(), liste.nElements);
}

shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const
{
	for (const Film* film : enSpan()) {
		for (shared_ptr<Acteur> acteur : spanListeActeurs(film->acteurs)) {
			if (acteur->nom == nomActeur)
				return acteur;
		}
	}
	return nullptr;
}


shared_ptr<Acteur> lireActeur(istream& fichier
, ListeFilms& listeFilms
)
{
	Acteur acteur = {};
	acteur.nom            = lireString(fichier);
	acteur.anneeNaissance = lireUint16 (fichier);
	acteur.sexe           = lireUint8  (fichier);

	shared_ptr<Acteur> acteurExistant = listeFilms.trouverActeur(acteur.nom);
	if (acteurExistant != nullptr)
		return acteurExistant;
	else {
		cout << "Création Acteur " << acteur.nom << endl;
		return make_shared<Acteur>(acteur);
	}

}

Film* lireFilm(istream& fichier
, ListeFilms& listeFilms
)
{
	Film* film = new Film;
	film->titre       = lireString(fichier);
	film->realisateur = lireString(fichier);
	film->anneeSortie = lireUint16 (fichier);
	film->recette     = lireUint16 (fichier);
	film->acteurs.nElements = lireUint8 (fichier);
	film->acteurs.elements = make_unique<shared_ptr<Acteur>[]>(film->acteurs.nElements);
	cout << "Création Film " << film->titre << endl;
	//filmp->acteurs.elements = new Acteur*[filmp->acteurs.nElements];

	for (shared_ptr<Acteur>& acteur : spanListeActeurs(film->acteurs)) {
		acteur = lireActeur(fichier, listeFilms);


	}

	return film;

}

ListeFilms::ListeFilms(const string& nomFichier) : possedeLesFilms_(true)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);
	
	int nElements = lireUint16(fichier);

	for ([[maybe_unused]] int i : range(nElements)) { 
		ajouterFilm(lireFilm(fichier, *this)); 
	}

}

void detruireActeur(shared_ptr<Acteur> acteur)
{
	cout << "Destruction Acteur " << acteur->nom << endl;
	delete &acteur;
}

void detruireFilm(Film* film)
{
	cout << "Destruction Film " << film->titre << endl;
	delete film;
}

ListeFilms::~ListeFilms()
{
	if (possedeLesFilms_)
		for (Film* film : enSpan())
			detruireFilm(film);
	delete[] elements;
}
//]

void afficherActeur(const Acteur& acteur)
{
	cout << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

void afficherFilm(const Film& film)
{
	cout << "Titre: " << film.titre << endl;
	cout << "  Réalisateur: " << film.realisateur << "  Année :" << film.anneeSortie << endl;
	cout << "  Recette: " << film.recette << "M$" << endl;

	cout << "Acteurs:" << endl;
	for (const shared_ptr<Acteur> acteur : spanListeActeurs(film.acteurs))
		afficherActeur(*acteur);
}
//]

void afficherListeFilms(const ListeFilms& listeFilms)
{

	static const string ligneDeSeparation = 
		"\033[32m────────────────────────────────────────\033[0m\n";

	cout << ligneDeSeparation;

	for (const Film* film : listeFilms.enSpan()) {

		afficherFilm(*film);

		cout << ligneDeSeparation;
	}
}
/*
void afficherFilmographieActeur(const ListeFilms& listeFilms, const string& nomActeur)
{
	const shared_ptr<Acteur> acteur = 
		listeFilms.trouverActeur(nomActeur);
	if (acteur == nullptr)
		cout << "Aucun acteur de ce nom" << endl;
	else
		afficherListeFilms(acteur->joueDans);
}
*/
int main()
{
	#ifdef VERIFICATION_ALLOCATION_INCLUS
	bibliotheque_cours::VerifierFuitesAllocations verifierFuitesAllocations;
	#endif
	bibliotheque_cours::activerCouleursAnsi(); 
	//int* fuite = new int; 

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";


	ListeFilms listeFilms("films.bin");
	
	cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;
	
	afficherFilm(*listeFilms.enSpan()[0]);
	

	cout << ligneDeSeparation << "Les films sont:" << endl;
	
	afficherListeFilms(listeFilms);
	
	listeFilms.trouverActeur("Benedict Cumberbatch")->anneeNaissance = 1976;
	
	cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << endl;
	
	//afficherFilmographieActeur(listeFilms, "Benedict Cumberbatch");
	
	detruireFilm(listeFilms.enSpan()[0]);
	listeFilms.enleverFilm(listeFilms.enSpan()[0]);


	cout << ligneDeSeparation << "Les films sont maintenant:" << endl;

	afficherListeFilms(listeFilms);

}
