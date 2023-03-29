//*** Solutionnaire version 2, sans les //[ //] au bon endroit car le code est assez différent du code fourni.
#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures_solutionnaire_2.hpp"      // Structures de données pour la collection de films en mémoire.
#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "memory"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include <vector>
#include "cppitertools/range.hpp"
#include "gsl/span"
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).
#include <sstream>
#include <forward_list>
#include <list>
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
	vector<shared_ptr<Film>> nouvelleListe;


	nElements = min(nouvelleCapacite, nElements);
	for (int i : range(nElements))
		nouvelleListe[i] = elements[i];
	elements = nouvelleListe;
	capacite = nouvelleCapacite;
}

void ListeFilms::ajouterFilm(shared_ptr<Film> film){
	if (nElements == capacite)
		changeDimension(max(1, capacite * 2));
	elements[nElements++] = film;
}


void ListeFilms::enleverFilm(const shared_ptr<Film> film) {
	for (shared_ptr<Film> element : elements) {
		if (element == film) {
			if (nElements > 1)
				element = elements[nElements - 1];
			nElements--;
			return;
		}
	}
}


shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const{
	for (const shared_ptr<Film> film : elements) {
		for (shared_ptr<Acteur> acteur : film->acteurs.elements) {
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

shared_ptr<Film> lireFilm(istream& fichier, ListeFilms& listeFilms){
	shared_ptr<Film> film(new Film);
	film->titre = lireString(fichier);
	film->realisateur = lireString(fichier);
	film->anneeSortie = lireUint16(fichier);
	film->recette = lireUint16(fichier);
	film->acteurs.nElements = lireUint8(fichier);
	film->acteurs.elements = vector<shared_ptr<Acteur>>(film->acteurs.nElements);
	cout << "Création Film " << film->titre << endl;
	for (shared_ptr<Acteur>& acteur : film->acteurs.elements) {
		acteur = lireActeur(fichier, listeFilms);
	}
	return film;
}

ListeFilms::ListeFilms(const string& nomFichier) : possedeLesFilms_(true){
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElement = lireUint16(fichier);

	for ([[maybe_unused]] int i : range(nElement)) {
		elements.push_back(lireFilm(fichier, *this));
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


/*
void afficherListeFilms(const ListeFilms& listeFilms) {

	static const string ligneDeSeparation =
		"\033[32m────────────────────────────────────────\033[0m\n";

	cout << ligneDeSeparation;

	for (const shared_ptr<Film> film : listeFilms.avoirElementsConst() ) {

		cout << *film;

		cout << ligneDeSeparation;
	}
}
*/


template<typename T>
void afficherListeFilms(const T& biblio) {
	static const string ligneDeSeparation =
		"\033[32m────────────────────────────────────────\033[0m\n";

	cout << ligneDeSeparation;

	for (const shared_ptr<Item> film : biblio) {

		cout << *film;

		cout << ligneDeSeparation;
	}
}


Film::Film() {
	titre = "PasDeTitre";
	realisateur = "PasDeRealisateur"; 
	anneeSortie = 0;
	recette = 0;
}

Film::Film(const Film& autreFilm) :  realisateur(autreFilm.realisateur), recette(autreFilm.recette) {
	titre = autreFilm.titre;
	anneeSortie = autreFilm.anneeSortie;
	acteurs.nElements = autreFilm.acteurs.nElements;
	acteurs.capacite = autreFilm.acteurs.capacite;
	acteurs.elements = vector<shared_ptr<Acteur>>(acteurs.nElements);
	*this = autreFilm;
}

Film& Film::operator= (const Film& autreFilm) {
	for (int i : range(acteurs.nElements)) {
		acteurs.elements[i] = autreFilm.acteurs.elements[i];
	}
	return *this;
}


template <typename Critere>
shared_ptr<Film> ListeFilms::rechercherCritere(const Critere critere) {
	for (Film* elem : this->elements) {
		if (critere(elem))
			return elem;
	}
	return nullptr;
}

template <typename Element>
Liste<Element>::Liste() {
	capacite = 0;
	nElements = 0;

	vector<shared_ptr<Element>> elementss;
	for (auto& elem : elementss) {
		elem = make_shared<Element>();
	}
	elements = elementss;
}


Livre::Livre(string titree, int anneSortie, string auteure, int copiVendues, int nombrePage) {
	titre = titree;
	anneeSortie = anneSortie;
	auteur = auteure;
	copieVendues = copiVendues;
	nombreDePage = nombrePage;
}


vector<shared_ptr<Item>> creerBibliotheque(ListeFilms& listeFilm, vector<shared_ptr<Livre>>& listeLivre) {
	vector<shared_ptr<Item>> vecteur;
	for (shared_ptr<Film> elem : listeFilm.avoirElements()) {
		vecteur.push_back(elem);
	}
	vecteur.insert(vecteur.end(), listeLivre.begin(), listeLivre.end());
	return vecteur;
}


vector<shared_ptr<Livre>> lireLivre(const string& nomFichier) {
	ifstream fichierLivre;
	fichierLivre.open(nomFichier);
	string texte;
	vector<string> livreDansFichier;
	vector<shared_ptr<Livre>> listeLivre;
	cout << fichierLivre.is_open();
	if (fichierLivre.is_open()) {
		while(getline(fichierLivre, texte)){
			livreDansFichier.push_back(texte);
		}
		unsigned int i = 0;
		for (i; i < livreDansFichier.size(); i++) {
			texte = livreDansFichier[i];
			int compte = 0;
			shared_ptr<Livre> livreTemp(new Livre);
			string donneTemp;
			int j = 0;
			for (j;; j++) {
				if (texte[j] == '\t' || texte[j] == '\0') {
					if (compte == 0) {
						livreTemp->modifierTitre(donneTemp);
						compte++;
					}
					else if (compte == 1) {
						livreTemp->modifierAnneeSortie(donneTemp);
						compte++;
					}
					else if (compte == 2) {
						livreTemp->modifierAuteur(donneTemp);
						compte++;
					}
					else if (compte == 3) {
						livreTemp->modifierCopieVendues(donneTemp);
						compte++;
					}
					else if (compte == 4) {
						livreTemp->modifierNombreDePage(donneTemp);
						compte++;
					}
					donneTemp = "";
				}
				else if (texte[j] == 34) continue;
				else {
					donneTemp += texte[j];
				}
				if (compte == 5) {
					listeLivre.push_back(livreTemp);
					break;
				}
			}
		}
	}
	else { cout << "probleme"; }
	return listeLivre;
}

ostream& operator<<(ostream& o, const Acteur acteur) {
	return o << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}


ostream& operator<< (ostream& o, const Film& film) {
	o << "  Réalisateur: " << film.avoirRealisateur()  << endl;
	o << "  Recette: " << film.avoirRecette() << "M$" << endl;

	o << "Acteurs:" << endl;
	for (const shared_ptr<Acteur>& acteur : film.avoirActeurs().elements) {
		o << *acteur;
	}
	return o;
}


ostream& operator<<(ostream& os,Item& item) {
	item.afficher();
	return os;
}
ostream& operator<<(ostream& os, FilmLivre& filmlivre) {
	filmlivre.afficher();
	return os;
}

void Item::afficher() const{
	cout << "Titre: " << titre << endl << "Annee de sortie: " << anneeSortie << endl;
}

void Film::afficher() const {
	Item::afficher();
	cout << "  Réalisateur: " << realisateur << endl
	 << "  Recette: " << recette << "M$" << endl;

	cout << "Acteurs: " << endl;
	for (const shared_ptr<Acteur>& acteur : acteurs.elements) {
		cout << *acteur;
	}
}

void Livre::afficher() const {
	Item::afficher();
	cout << "Auteur: " << auteur << endl
		<< "Nombre de pages: " << nombreDePage << "  Copies Vendues: " << copieVendues << endl;
}

void FilmLivre::afficher() const {
	Film::afficher();
	cout << "Auteur: " << auteur << endl << "Nombre de pages: " << nombreDePage << "  Copies Vendues: " << copieVendues << endl;
}

FilmLivre::FilmLivre(shared_ptr<Item> film, shared_ptr<Item> livre) {
	Film* nouvFilm = dynamic_cast<Film*>(&*film);
	Livre* nouvLivre = dynamic_cast<Livre*>(&*livre); // livre ne sera jamais partiellement construit grâce au constructeur par défaut
	if (nouvFilm != 0) {
		if (nouvLivre != 0) {
			titre = nouvFilm->avoirTitre();
			anneeSortie = nouvFilm->avoirAnneSortie();
			auteur = nouvLivre->avoirAuteur();
			copieVendues = nouvLivre->avoirCopieVendues();
			nombreDePage = nouvLivre->avoirNombreDePage();
			realisateur = nouvFilm->avoirRealisateur();
			acteurs = nouvFilm->avoirActeurs();
			recette = nouvFilm->avoirRecette();
		}
	}
}

forward_list<shared_ptr<Item>> enForwardList(vector<shared_ptr<Item>>& liste) { // 1.1
	forward_list<shared_ptr<Item>> nouvListe;
	auto it = nouvListe.before_begin();

	for (unsigned int i = 0 ; i < liste.size() ; ++i) {
		nouvListe.insert_after(it, liste[i]);
		it++;
	}
	return nouvListe;
}

forward_list<shared_ptr<Item>> enForwardListInverser(vector<shared_ptr<Item>>& liste) { // 1.2
	forward_list<shared_ptr<Item>> nouvListe;

	for (unsigned int i = 0; i < liste.size(); ++i) {
		nouvListe.push_front(liste[i]);
	}
	return nouvListe;
}
/*



forward_list<shared_ptr<Item>> forwardAForward(forward_list<shared_ptr<Item>>& liste) { // 1.3
	forward_list<shared_ptr<Item>> nouvListe;
	auto it = liste.begin();
	auto itMax = liste.end();
	auto iterateur2 = nouvListe.before_begin();
	
	while (it != itMax) {

		nouvListe.insert_after(iterateur2, *it);
		it++;
		iterateur2++;
		cout << **it << endl;
	}
//	cout << "on se rend au return " << endl;
	return nouvListe;
}

*/


vector<shared_ptr<Item>> enVecteurInverse(forward_list<shared_ptr<Item>>& liste) { // 1.4 Ordre : O(2*n)
	vector<shared_ptr<Item>> vectorInverse;
	size_t tailleListe = 0;
	auto it = liste.begin();
	
	while (*it != nullptr) {
		tailleListe++;
		it++;
	}
	it = liste.begin();
	vectorInverse.reserve(tailleListe);
	for (auto i : range(tailleListe)) {
		vectorInverse[tailleListe - i] = *it;
		it++;
	}
	
	return vectorInverse;
}

int main(){

#ifdef VERIFICATION_ALLOCATION_INCLUS
	bibliotheque_cours::VerifierFuitesAllocations verifierFuitesAllocations;
#endif
	bibliotheque_cours::activerCouleursAnsi();
	 


	string fichierLivre = "livres.txt";
	vector<shared_ptr<Livre>> listeLivre= lireLivre(fichierLivre);


	ListeFilms listeFilms("films.bin");
	vector<shared_ptr<Item>> biblio = creerBibliotheque(listeFilms, listeLivre);


	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";
	
	cout << ligneDeSeparation;

	afficherListeFilms(biblio);

	forward_list<shared_ptr<Item>> bilioEnListe = enForwardList(biblio);
	forward_list<shared_ptr<Item>> bilioEnListeInverser = enForwardListInverser(biblio);
//	forward_list<shared_ptr<Item>> biblioEnForwardEnForward = forwardAForward(bilioEnListe);

	cout << "En forward liste" << '\n';
	cout << ligneDeSeparation;


	for (shared_ptr<Item> item : bilioEnListe) {
		cout << *item;
		cout << ligneDeSeparation;
	}

	for (shared_ptr<Item> item : bilioEnListeInverser) {
		cout << *item;
		cout << ligneDeSeparation;
	}

	cout << "CREATION D'UN FILM-LIVRE : " << endl;
	FilmLivre hobbit(biblio[4], biblio[9]);
	cout << hobbit;
	return 0;

}
