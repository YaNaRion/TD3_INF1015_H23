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
	vector<shared_ptr<Film>> nouvelleListe;//Film * [nouvelleCapacite];


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

//span<Film*> ListeFilms::enSpan() const { return span(elements, nElements); }

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

//span<shared_ptr<Acteur>> spanListeActeurs(const Liste<Acteur>& liste) {
//	return span(liste.elements);
//}

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
		//ajouterFilm(lireFilm(fichier, *this));
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

//ListeFilms::~ListeFilms(){
//	if (possedeLesFilms_)
//		for (Film* film : enSpan())
//			detruireFilm(film);
//	delete[] elements;
//}
//]



void afficherListeFilms(const ListeFilms& listeFilms) {

	static const string ligneDeSeparation =
		"\033[32m────────────────────────────────────────\033[0m\n";

	cout << ligneDeSeparation;

	for (const shared_ptr<Film> film : listeFilms.avoirElementsConst() ) {

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

template <class Element>
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
//	span<shared_ptr<Film>> spanListeFilm = ;
	vector<shared_ptr<Item>> vecteur;
	for (shared_ptr<Film> elem : listeFilm.avoirElements()) {
		vecteur.push_back(elem);
		cout << "ajout de " << elem << endl;
	}
	vecteur.insert(vecteur.end(), listeLivre.begin(), listeLivre.end());
	return vecteur;
}


vector<shared_ptr<Livre>> lireLivre(const string& nomFichier) {
	ifstream fichierLivre;
	cout << nomFichier;
	fichierLivre.open(nomFichier);
	string texte;
	vector<string> livreDansFichier;
	vector<shared_ptr<Livre>> listeLivre;
	cout << fichierLivre.is_open();
	if (fichierLivre.is_open()) {
		while(getline(fichierLivre, texte)){
			livreDansFichier.push_back(texte);
		}
		int i = 0;
		for (i; i < livreDansFichier.size(); i++) {
			texte = livreDansFichier[i];
			int compte = 0;
			shared_ptr<Livre> livreTemp(new Livre);
			string donneTemp;
			int j = 0;
			int longeurTexte = texte.length();
			for (j;; j++) {
				if (texte[j] == '\t' || texte[j] == '\0') {
					if (compte == 0) {
						livreTemp->modifierTitre(donneTemp);
						//cout << "jai mis le titre\t";
						compte++;
					}
					else if (compte == 1) {
						livreTemp->modifierAnneeSortie(donneTemp);
						//cout << "jai mis lanne\t";
						compte++;
					}
					else if (compte == 2) {
						livreTemp->modifierAuteur(donneTemp);
						//cout << "jai mis lauteur\t";
						compte++;
					}
					else if (compte == 3) {
						livreTemp->modifierCopieVendues(donneTemp);
						//cout << "jai mis les copies\t";
						compte++;
					}
					else if (compte == 4) {
						livreTemp->modifierNombreDePage(donneTemp);
						//cout << "jai mis les pages\t";
						compte++;
					}
					donneTemp = "";
				}
				else if (texte[j] == 34) continue;
				else {
					donneTemp += texte[j];
				}
				if (compte == 5) {
					cout << "tous va bien" << endl;
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

//]
ostream& operator<< (ostream& o, const Film& film) {
//	o << "Titre: " << film.avoirTitre() << endl << "  Année :" << film.avoirAnneSortie() << endl;
//	o << Item(film);
	o << "  Réalisateur: " << film.avoirRealisateur()  << endl;
	o << "  Recette: " << film.avoirRecette() << "M$" << endl;

	o << "Acteurs:" << endl;
	for (const shared_ptr<Acteur>& acteur : film.avoirActeurs().elements) {
		o << *acteur;
	}

	return o;
}


ostream& operator<<(ostream& os,const Item& item) {
	item.afficher();
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


FilmLivre::FilmLivre(shared_ptr<Item> film, shared_ptr<Item> livre) {
	Film* nouvFilm = dynamic_cast<Film*>(&*film);
	Livre* nouvLivre = dynamic_cast<Livre*>(&*livre);
	if (nouvFilm != 0) {
		if (nouvLivre != 0) {
			//cout << endl << "titre nouvFilm " << nouvFilm->avoirTitre();
			modifierTitre(nouvFilm->avoirTitre());
			modifierAnneeSortie(nouvFilm->avoirAnneSortie());
			modifierAuteur(nouvLivre->avoirAuteur());
			modifierCopieVendues(nouvLivre->avoirCopieVendues());
			modifierNombreDePage(nouvLivre->avoirNombreDePage());
			modifierRealisateur(nouvFilm->avoirRealisateur());
			modifierActeur(nouvFilm->avoirActeurs());
			modifierRecette(nouvFilm->avoirRecette());
		}
	}
}

void FilmLivre::afficher() const {
	Film::afficher();
	cout << "Auteur: " << auteur << endl << "Nombre de pages: " << nombreDePage << "  Copies Vendues: " << copieVendues << endl;
}

int main(){

#ifdef VERIFICATION_ALLOCATION_INCLUS
	bibliotheque_cours::VerifierFuitesAllocations verifierFuitesAllocations;
#endif
	bibliotheque_cours::activerCouleursAnsi();



	string fichierLivre = "livres.txt";
	vector<shared_ptr<Livre>> listeLivre= lireLivre(fichierLivre);
//	cout << *listeLivre[0];
	//cout << listeLivre[0]->avoirTitre();

//	return 0;

	ListeFilms listeFilms("films.bin");
	vector<shared_ptr<Item>> biblio = creerBibliotheque(listeFilms, listeLivre);
//	cout << *biblio[9];
	FilmLivre hobbit(biblio[4], biblio[9]);
//	cout << hobbit;
	hobbit.afficher();

//	span<Film*> spanListeFilm = listeFilms.enSpan();
//	cout << *spanListeFilm[0];
//	afficherListeFilms(listeFilms);


	

	//int* fuite = new int; 
/*
	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	ListeFilms listeFilms("films.bin");



	cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;

	//afficherFilm(*listeFilms.enSpan()[0]);
	span<Film*> spanListeFilm = listeFilms.enSpan();
	cout << *spanListeFilm[0];

	cout << ligneDeSeparation << "Les films sont:" << endl;

	afficherListeFilms(listeFilms);

	listeFilms.trouverActeur("Benedict Cumberbatch")->anneeNaissance = 1976;

	cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << endl;

	//afficherFilmographieActeur(listeFilms, "Benedict Cumberbatch");


	Film skylien = *listeFilms.enSpan()[0];
	skylien.avoirTitreNonConst() = "Skylien";
	skylien.avoirActeursNonConst().elements[0] = spanListeFilm[1]->avoirActeursNonConst().elements[0];
	skylien.avoirActeurs().elements[0]->nom = "Daniel Wroughton Craig";

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
	*/
}
