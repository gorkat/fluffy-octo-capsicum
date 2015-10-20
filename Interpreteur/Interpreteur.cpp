#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(NULL) {
}

void Interpreteur::analyse() {
  m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
  static char messageWhat[256];
  if (m_lecteur.getSymbole() != symboleAttendu) {
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(),
            symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
  }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
  tester(symboleAttendu);
  m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
  // Lève une exception contenant le message et le symbole courant trouvé
  // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
  static char messageWhat[256];
  sprintf(messageWhat,
          "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
          m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
  throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
  // <programme> ::= procedure principale() debut <seqInst> fin FIN_FICHIER
  testerEtAvancer("procedure");
  testerEtAvancer("principale");
  testerEtAvancer("(");
  testerEtAvancer(")");
  Noeud* sequence = seqInst();
  testerEtAvancer("finproc");
  tester("<FINDEFICHIER>");
  return sequence;
}

Noeud* Interpreteur::seqInst() {
  // <seqInst> ::= <inst> ; { <inst> ; }
  NoeudSeqInst* sequence = new NoeudSeqInst();
  do {
    sequence->ajoute(inst());
  } while (m_lecteur.getSymbole() == "<VARIABLE>" || 
          m_lecteur.getSymbole() == "si" || 
          m_lecteur.getSymbole() == "tantque" || 
          m_lecteur.getSymbole() == "pour" || 
          m_lecteur.getSymbole() =="ecrire" ||
          m_lecteur.getSymbole() =="repeter" ||
          m_lecteur.getSymbole() =="lire");
  // tant que le symbole courant est un debut possible d'instruction...
  return sequence;
}

Noeud* Interpreteur::inst() {
  // <inst> ::= <affectation>  ; | <inst_condi>
  if (m_lecteur.getSymbole() == "<VARIABLE>") {
    Noeud *affect = affectation();
    testerEtAvancer(";");
    return affect;
  }
  else if (m_lecteur.getSymbole() == "si")
    return instSi();
  else if (m_lecteur.getSymbole() == "tantque")
    return instTantQue();
  else if (m_lecteur.getSymbole() == "pour")
      return instPour();
  else if (m_lecteur.getSymbole()=="ecrire")
      return instEcrire();
  else if (m_lecteur.getSymbole()=="repeter")
      return instRepeter();
  else if (m_lecteur.getSymbole()=="lire"){
      return instLire();
  }
  else erreur("Instruction incorrecte");
}

Noeud* Interpreteur::affectation() {
  // <affectation> ::= <variable> = <expression> 
  tester("<VARIABLE>");
  Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // la variable est ajoutée à la table
  m_lecteur.avancer();
  testerEtAvancer("=");
  Noeud* exp = expression(); // On mémorise l'expression trouvée
  return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression() {
  // <expression> ::= <facteur> { <opBinaire> <facteur> }
  //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
  Noeud* fact = facteur();
  while ( m_lecteur.getSymbole() == "+"  || m_lecteur.getSymbole() == "-"  ||
          m_lecteur.getSymbole() == "*"  || m_lecteur.getSymbole() == "/"  ||
          m_lecteur.getSymbole() == "<"  || m_lecteur.getSymbole() == "<=" ||
          m_lecteur.getSymbole() == ">"  || m_lecteur.getSymbole() == ">=" ||
          m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
          m_lecteur.getSymbole() == "et" || m_lecteur.getSymbole() == "ou"   ) {
    Symbole operateur = m_lecteur.getSymbole(); // On stocke le symbole de l'opérateur
    m_lecteur.avancer();
    Noeud* factDroit = facteur(); // Lecture de l'operande droit
    fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // construction du noeud operateur
  }
  return fact; // on renvoie le dernier noeud trouvé ou construit
}

Noeud* Interpreteur::facteur() {
  // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
  Noeud* fact = NULL;
  if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
    fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
    m_lecteur.avancer();
  } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
  } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), NULL);
  } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
    m_lecteur.avancer();
    fact = expression();
    testerEtAvancer(")");
  } else
    erreur("Facteur incorrect");
  return fact;
}

Noeud* Interpreteur::instSi() {
  // <instSi> ::= si ( <condition> ) <seqInst> finsi
  vector <Noeud*> conditionsinonsi;  
  vector <Noeud*> sequencesinonsi;  
  
  testerEtAvancer("si");
  testerEtAvancer("(");
  conditionsinonsi.push_back(expression());
  testerEtAvancer(")");
  sequencesinonsi.push_back(seqInst());
  while(m_lecteur.getSymbole() =="sinonsi"){
      testerEtAvancer("sinonsi");
      testerEtAvancer("(");
      conditionsinonsi.push_back(expression());
      testerEtAvancer(")");
      sequencesinonsi.push_back(seqInst());
  }
  if(m_lecteur.getSymbole() =="sinon"){
      testerEtAvancer("sinon");
      sequencesinonsi.push_back(seqInst());
  }
  testerEtAvancer("finsi");
  return new NoeudInstSi(conditionsinonsi,sequencesinonsi);
}

Noeud* Interpreteur::instTantQue() {
  // <instSi> ::= si ( <condition> ) <seqInst> finsi
  testerEtAvancer("tantque");
  testerEtAvancer("(");
  Noeud* condition = expression();
  testerEtAvancer(")");
  Noeud* sequence = seqInst();
  testerEtAvancer("fintantque");
  return new NoeudInstTantQue(condition, sequence);
}

Noeud* Interpreteur::instPour() {
  // <instSi> ::= si ( <condition> ) <seqInst> finsi
  Noeud* condition1=NULL;  
  Noeud* condition3=NULL;
  
  testerEtAvancer("pour");
  testerEtAvancer("(");
  if(m_lecteur.getSymbole()!=";"){
      condition1 = affectation();
  }
  testerEtAvancer(";");
  Noeud* condition2 = expression();
  testerEtAvancer(";");
  if(m_lecteur.getSymbole()!=")"){
      condition3 = affectation();
  }
  testerEtAvancer(")");
  Noeud* sequence = seqInst();
  testerEtAvancer("finpour");
  return new NoeudInstPour(condition1,condition2,condition3, sequence);
}

Noeud* Interpreteur::instEcrire(){
    vector <Noeud*> message;
    testerEtAvancer("ecrire");
    testerEtAvancer("(");
    while(m_lecteur.getSymbole()!=")"){
        if(m_lecteur.getSymbole()=="<CHAINE>"){
            message.push_back(m_table.chercheAjoute(m_lecteur.getSymbole()));
            m_lecteur.avancer();
        }
        else{
            message.push_back(expression());
        }  
        if(m_lecteur.getSymbole()==","){
            m_lecteur.avancer();
        }
    }       
    testerEtAvancer(")");
    testerEtAvancer(";");
    return new NoeudInstEcrire(message);
}

Noeud* Interpreteur::instRepeter(){
    testerEtAvancer("repeter");
    Noeud* instructions = seqInst();
    testerEtAvancer("jusqua");
    testerEtAvancer("(");
    Noeud* condition = expression();
    testerEtAvancer(")");
    return new NoeudRepeter(instructions,condition);
}

Noeud* Interpreteur::instLire(){
       vector<Noeud*> variables;
       testerEtAvancer("lire");
       testerEtAvancer("(");
       while(m_lecteur.getSymbole()!=")"){
       if(m_lecteur.getSymbole()=="<VARIABLE>"){
           variables.push_back(m_table.chercheAjoute(m_lecteur.getSymbole())); // on ajoute la variable à la table
           m_lecteur.avancer();
       }
       if(m_lecteur.getSymbole()==","){
           testerEtAvancer(",");
       }
       }
       testerEtAvancer(")");
       testerEtAvancer(";");
       return new NoeudInstLire(variables);
}

//////////////////////////////////////////////////
///////////// Traduction en C++ //////////////////
//////////////////////////////////////////////////

void Interpreteur::traduitenCpp(ostream & cout, unsigned int indentation) const{
      cout <<setw(4*indentation)<<""<<"int main() {"<<endl;
      for(int i = 0; i < m_table.getTaille(); i++){
          if(m_table[i].operator ==("<VARIABLE>")) {
              cout << setw(4*indentation) << "" << "int " << m_table[i].getChaine() << ";" << endl;
          }
      }
    // Début d’un programme Java
    // Ecrire en Java la déclaration des variables présentes dans le programme... 
    // ... variables dont on retrouvera le nom en parcourant la table des symboles ! 
    // Par exemple, si le programme contient i,j,k, il faudra écrire : int i; int j; int k; ... 
    getArbre()->traduitenCpp(cout,indentation+1);
    // lance l'opération traduitenJava sur la racine
    cout <<setw(4*(indentation+1))<<""<<"return 0;"<<endl ; 
    cout <<setw(4*indentation)<<"}" <<endl ; 
    // Fin d’un programme 
}


