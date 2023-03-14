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
#include <sstream>
using namespace iter;
using namespace gsl;

#pragma endregion//}

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{

UInt8 lireUint8(istream& fichier){
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier){
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier){
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}


void ListeFilms::changeDimension(int nouvelleCapacite){
	Film** nouvelleListe = new Film * [nouvelleCapacite];

	if (elements != nullptr) {
		nElements = min(nouvelleCapacite, nElements);
		for (int i : range(nElements))
			nouvelleListe[i] = elements[i];
		delete[] elements;
	}

	elements = nouvelleListe;
	capacite = nouvelleCapacite;
}

void ListeFilms::ajouterFilm(Film* film){
	if (nElements == capacite)
		changeDimension(max(1, capacite * 2));
	elements[nElements++] = film;
}

span<Film*> ListeFilms::enSpan() const { return span(elements, nElements); }

void ListeFilms::enleverFilm(const Film* film) {
	for (Film*& element : enSpan()) {
		if (element == film) {
			if (nElements > 1)
				element = elements[nElements - 1];
			nElements--;
			return;
		}
	}
}

span<shared_ptr<Acteur>> spanListeActeurs(const Liste<Acteur>& liste) {
	return span<shared_ptr<Acteur>>(liste.elements.get(), liste.nElements);
}

shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const{
	for (const Film* film : enSpan()) {
		for (shared_ptr<Acteur> acteur : spanListeActeurs(film->acteurs)) {
			if (acteur->nom == nomActeur)
				return acteur;
		}
	}
	return nullptr;
}


shared_ptr<Acteur> lireActeur(istream& fichier, ListeFilms& listeFilms){
	Acteur acteur = {};
	acteur.nom = lireString(fichier);
	acteur.anneeNaissance = lireUint16(fichier);
	acteur.sexe = lireUint8(fichier);

	shared_ptr<Acteur> acteurExistant = listeFilms.trouverActeur(acteur.nom);
	if (acteurExistant != nullptr)
		return acteurExistant;
	else {
		cout << "Création Acteur " << acteur.nom << endl;
		return make_shared<Acteur>(acteur);
	}

}

Film* lireFilm(istream& fichier, ListeFilms& listeFilms){
	Film* film = new Film;
	film->titre = lireString(fichier);
	film->realisateur = lireString(fichier);
	film->anneeSortie = lireUint16(fichier);
	film->recette = lireUint16(fichier);
	film->acteurs.nElements = lireUint8(fichier);
	film->acteurs.elements = make_unique<shared_ptr<Acteur>[]>(film->acteurs.nElements);
	cout << "Création Film " << film->titre << endl;
	for (shared_ptr<Acteur>& acteur : spanListeActeurs(film->acteurs)) {
		acteur = lireActeur(fichier, listeFilms);
	}
	return film;
}

ListeFilms::ListeFilms(const string& nomFichier) : possedeLesFilms_(true){
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElement = lireUint16(fichier);

	for ([[maybe_unused]] int i : range(nElement)) {
		ajouterFilm(lireFilm(fichier, *this));
	}

}

void detruireActeur(shared_ptr<Acteur> acteur){
	cout << "Destruction Acteur " << acteur->nom << endl;
	delete& acteur;
}

void detruireFilm(Film* film){
	cout << "Destruction Film " << film->avoirTitre() << endl;
	delete film;
}

ListeFilms::~ListeFilms(){
	if (possedeLesFilms_)
		for (Film* film : enSpan())
			detruireFilm(film);
	delete[] elements;
}
//]


ostream& operator<<(ostream& o, const Acteur& acteur) {
	return o << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

//]
ostream& operator<< (ostream& o, const Film& film) {
	o << "Titre: " << film.avoirTitre() << endl;
	o << "  Réalisateur: " << film.avoirRealisateur() << "  Année :" << film.avoirAnneSortie() << endl;
	o << "  Recette: " << film.avoirRecette() << "M$" << endl;

	o << "Acteurs:" << endl;
	for (const shared_ptr<Acteur> acteur : spanListeActeurs(film.avoirActeur()))
		o << *acteur;
	return o;
}

void afficherListeFilms(const ListeFilms& listeFilms) {

	static const string ligneDeSeparation =
		"\033[32m────────────────────────────────────────\033[0m\n";

	cout << ligneDeSeparation;

	for (const Film* film : listeFilms.enSpan()) {

		cout << *film;

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

Film::Film() {
	titre = "PasDeTitre";
	realisateur = "PasDeRealisateur"; // Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
	anneeSortie = 0;
	recette = 0;
}

Film::Film(const Film& autreFilm) :  realisateur(autreFilm.realisateur), recette(autreFilm.recette) {
	titre = autreFilm.titre;
	anneeSortie = autreFilm.anneeSortie;
	acteurs.nElements = autreFilm.acteurs.nElements;
	acteurs.capacite = autreFilm.acteurs.capacite;
	acteurs.elements = make_unique<shared_ptr<Acteur>[]>(acteurs.nElements);
	*this = autreFilm;
}

Film& Film::operator= (const Film& autreFilm) {
	for (int i : range(acteurs.nElements)) {
		acteurs.elements[i] = autreFilm.acteurs.elements[i];
	}
	return *this;
}


template <typename Critere>
Film* ListeFilms::rechercherCritere(const Critere critere) {
	for (Film* elem : this->enSpan()) {
		if (critere(elem))
			return elem;
	}
	return nullptr;
}

int main(){
#ifdef VERIFICATION_ALLOCATION_INCLUS
	bibliotheque_cours::VerifierFuitesAllocations verifierFuitesAllocations;
#endif
	bibliotheque_cours::activerCouleursAnsi();
	//int* fuite = new int; 

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	ListeFilms listeFilms("films.bin");

	cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;

	//afficherFilm(*listeFilms.enSpan()[0]);
	span<Film*> spanListeFilm = listeFilms.enSpan();
	//cout << *spanListeFilm[0];

	cout << ligneDeSeparation << "Les films sont:" << endl;

	afficherListeFilms(listeFilms);

	listeFilms.trouverActeur("Benedict Cumberbatch")->anneeNaissance = 1976;

	cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << endl;

	//afficherFilmographieActeur(listeFilms, "Benedict Cumberbatch");


	Film skylien = *listeFilms.enSpan()[0];
	skylien.avoirTitreNonConst() = "Skylien";
	skylien.avoirActeur().elements[0] = spanListeFilm[1]->avoirActeur().elements[0];
	skylien.avoirActeur().elements[0]->nom = "Daniel Wroughton Craig";

	cout << *spanListeFilm[0] << endl << skylien << endl << *spanListeFilm[1] << endl;

	detruireFilm(spanListeFilm[0]);
	listeFilms.enleverFilm(spanListeFilm[0]);


	cout << ligneDeSeparation << "Les films sont maintenant:" << endl;

	afficherListeFilms(listeFilms);

	Liste<string> listeTextes = 2;
	listeTextes.elements[0] = make_shared<string>("Rome Total war");
	listeTextes.elements[1] = make_shared<string>("Medieval 2 total war");
	cout << *listeTextes.elements[1] << endl;
	Liste<string> listeTextes2 = listeTextes;
	listeTextes.elements[0] = make_shared<string>("Rome Remaster Total war");
	cout << *listeTextes.elements[0] << endl;
	cout << *listeTextes.elements[1] << endl;
	cout << *listeTextes2.elements[0] << endl;
	cout << *listeTextes2.elements[1] << endl;

	cout << *listeFilms.rechercherCritere([](Film* film) { return film->avoirRecette() == 955; });
}
