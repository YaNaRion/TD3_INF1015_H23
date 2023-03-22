#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <cassert>
#include <vector>
#include "gsl/span"
#include "memory"
using gsl::span;
using namespace std;

class Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class ListeFilms {
public:
	ListeFilms() = default;
	ListeFilms(const std::string& nomFichier);
	void ajouterFilm(shared_ptr<Film> film);
	void enleverFilm(const shared_ptr<Film> film);
	shared_ptr<Acteur> trouverActeur(const std::string& nomActeur) const;
	int size() const { return nElements; }
	template <typename Critere>
	shared_ptr<Film> rechercherCritere(const Critere critere);
	vector <shared_ptr<Film>> avoirElements() { return elements; }
	vector <shared_ptr<Film>> avoirElementsConst() const { return elements; }
private:
	void changeDimension(int nouvelleCapacite);
	int capacite = 0, nElements = 0;
	vector <shared_ptr<Film>> elements;
	bool possedeLesFilms_ = false; 
};

template <typename Element>
class Liste {
public:
	int capacite;
	int nElements;
	vector<shared_ptr<Element>> elements;
	Liste();
	Liste(int taille) : capacite(taille), nElements(taille), elements(taille) {}
	Liste(const Liste<Element>& listeACopier) : capacite(listeACopier.capacite), nElements(listeACopier.nElements), elements(listeACopier.elements) {}
};

using ListeActeurs = Liste<Acteur>;

class Item {
public:
	const string avoirTitre() const { return titre; }
	const int avoirAnneSortie() const { return anneeSortie; }
	void modifierTitre(string nouvTitre) { titre = nouvTitre; }
	void modifierAnneeSortie(string nouvAnneeSortie) { anneeSortie = stoi(nouvAnneeSortie); }
	void modifierAnneeSortie(int nouvAnneeSortie) { anneeSortie = nouvAnneeSortie; }
	virtual void afficher() const;
	friend ostream& operator<<(ostream&,Item&);
	friend class ListeFilms;
	friend shared_ptr<Film> lireFilm(istream& fichier, ListeFilms& listeFilms);

protected:
	string titre = "PasDeTitre";
	int anneeSortie = 0;

};

class Film : virtual public Item {
public:
	Film();
	Film(const Film& autreFilm);
	Film& operator= (const Film& autreFilm);
	//string avoirTitreNonConst() const{ return titre; }
	string avoirTitre() { return titre; }
	int avoirAnneSortieFilm() const { return anneeSortie; }
	const string avoirRealisateur() const { return realisateur; }
	const int avoirRecette() const { return recette; }
	const ListeActeurs avoirActeurs() const { return acteurs; }
	ListeActeurs avoirActeursNonConst() { return acteurs; }
	void modifierRealisateur(string nouvRealisateur) { realisateur = nouvRealisateur; }
	void modifierRecette(int nouvRecette) { recette = nouvRecette; }
	void modifierActeur(ListeActeurs nouvActeurs) { acteurs = nouvActeurs; }
	friend class ListeFilms;
	friend shared_ptr<Film> lireFilm(istream& fichier, ListeFilms& listeFilms);
	virtual void afficher() const;

protected:
	string realisateur = "PasDeRealisateur";
	int recette = 0;
	ListeActeurs acteurs;
};

class Livre : virtual public Item {
public : 
	Livre() = default;
	Livre(string titre, int anneeSortie, string auteur, int copieVendues, int nombrePage );
	const string avoirAuteur() const { return auteur; }
	const int avoirCopieVendues() const{ return copieVendues; }
	const int avoirNombreDePage() const { return nombreDePage; }
	void modifierAuteur(string nouvAuteur) { auteur = nouvAuteur; }
	void modifierCopieVendues(string nouvCopieVendues) { copieVendues = stoi(nouvCopieVendues); }
	void modifierCopieVendues(int nouvCopieVendues) { copieVendues = nouvCopieVendues; }
	void modifierNombreDePage(string nouvNombrePage) { nombreDePage = stoi(nouvNombrePage); }
	void modifierNombreDePage(int nouvNombrePage) { nombreDePage = nouvNombrePage; }
	virtual void afficher() const;


protected:
	string auteur = "PasDAuteur";
	int copieVendues = 0;
	int nombreDePage = 0;

};

class FilmLivre : public Film, public Livre {
public:
	FilmLivre(shared_ptr<Item> film, shared_ptr<Item> livre);
	virtual void afficher() const;
	friend ostream& operator<<(ostream&, FilmLivre&);
private:
};


struct Acteur
{
	std::string nom = "Pas de nom"; int anneeNaissance = 0; char sexe = 'x';
};
