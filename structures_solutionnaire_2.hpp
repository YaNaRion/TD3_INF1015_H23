#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <cassert>
#include "gsl/span"
#include "memory"
using gsl::span;
using namespace std;

struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class ListeFilms {
public:
	ListeFilms() = default;
	ListeFilms(const std::string& nomFichier);
	ListeFilms(const ListeFilms& l) { assert(l.elements == nullptr); } // Pas demandé dans l'énoncé, mais on veut s'assurer qu'on ne fait jamais de copie de liste, car la copie par défaut ne fait pas ce qu'on veut.  Donc on ne permet pas de copier une liste non vide (la copie de liste vide est utilisée dans la création d'un acteur).
	~ListeFilms();
	void ajouterFilm(Film* film);
	void enleverFilm(const Film* film);
	shared_ptr<Acteur> trouverActeur(const std::string& nomActeur) const;
	span<Film*> enSpan() const;
	int size() const { return nElements; }
	template <typename Critere>
	Film* rechercherCritere(const Critere critere);

private:
	void changeDimension(int nouvelleCapacite);
	int capacite = 0, nElements = 0;
	Film** elements = nullptr; // Pointeur vers un tableau de Film*, chaque Film* pointant vers un Film.
	bool possedeLesFilms_ = false; // Les films seront détruits avec la liste si elle les possède.
};

template <class Element>
class Liste {
public:
	int capacite = 0;
	int nElements = 0;
	unique_ptr<shared_ptr<Element>[]> elements;
	Liste() = default;
	Liste(int taille) : capacite(taille), nElements(taille), elements(new shared_ptr<Element>[taille]) {}
	Liste(const Liste<Element>& listeACopier) : capacite(listeACopier.capacite), nElements(listeACopier.nElements), elements(new shared_ptr<Element>[listeACopier.capacite]) {
		for (int i = 0; i < listeACopier.nElements; i++) {
			elements[i] = listeACopier.elements[i];
		}
	}
};

using ListeActeurs = Liste<Acteur>;


class Item {
public:
	const string avoirTitre() const { return titre; }
	const int avoirAnneSortie() const { return anneeSortie; }

private:
	string titre = "PasDeTitre";
	int anneeSortie = 0;

	friend class ListeFilms;
	friend class Film;
	friend Film* lireFilm(istream& fichier, ListeFilms& listeFilms);
};

class Film : public Item {
public:
	Film();
	Film(const Film& autreFilm);
	Film& operator= (const Film& autreFilm);
	string avoirTitreNonConst() { return titre; }
	const string avoirRealisateur() const { return realisateur; }
	const int avoirRecette() const { return recette; }
	const ListeActeurs avoirActeur() const { return acteurs; }
	friend class ListeFilms;
	friend Film* lireFilm(istream& fichier, ListeFilms& listeFilms);
private:
	string realisateur = "PasDeRealisateur";
	int recette = 0;
	ListeActeurs acteurs;
};

class Livre : public Item {
public : 
	const string avoirAuteur() const { return auteur; }
	const int avoirCopieVendues() const{ return copieVendues; }
	const int avoirNombreDePage() const { return nombreDePage; }
private:
	string auteur = "PasDAuteur";
	int copieVendues = 0;
	int nombreDePage = 0;

};

/*
struct Film
{
	Film();
	std::string titre = "PasDeTitre", realisateur = "PasDeRealisateur"; // Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
	int anneeSortie = 0, recette = 0; // Année de sortie et recette globale du film en millions de dollars
	ListeActeurs acteurs;
	Film(const Film& autreFilm);
	Film& operator= (const Film& autreFilm);

};
*/
struct Acteur
{
	std::string nom; int anneeNaissance; char sexe;
	//ListeFilms joueDans;
};
