#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
  for (unsigned int i = 0; i < m_instructions.size(); i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
  if (instruction!=NULL) m_instructions.push_back(instruction);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
  int valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
  int og, od, valeur;
  if (m_operandeGauche != NULL) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != NULL) od = m_operandeDroit->executer(); // On évalue l'opérande droit
  // Et on combine les deux opérandes en fonctions de l'opérateur
  if (this->m_operateur == "+") valeur = (og + od);
  else if (this->m_operateur == "-") valeur = (og - od);
  else if (this->m_operateur == "*") valeur = (og * od);
  else if (this->m_operateur == "==") valeur = (og == od);
  else if (this->m_operateur == "!=") valeur = (og != od);
  else if (this->m_operateur == "<") valeur = (og < od);
  else if (this->m_operateur == ">") valeur = (og > od);
  else if (this->m_operateur == "<=") valeur = (og <= od);
  else if (this->m_operateur == ">=") valeur = (og >= od);
  else if (this->m_operateur == "et") valeur = (og && od);
  else if (this->m_operateur == "ou") valeur = (og || od);
  else if (this->m_operateur == "non") valeur = (!og);
  else if (this->m_operateur == "/") {
    if (od == 0) throw DivParZeroException();
    valeur = og / od;
  }
  return valeur; // On retourne la valeur calculée
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(Noeud* conditionsi, Noeud* sequencesi,vector <Noeud*> conditionsinonsi,vector <Noeud*> sequeuncesinonsi,Noeud* sequencesinon)
: m_conditionsi(conditionsi), m_sequencesi(sequencesi),m_conditionsinonsi(conditionsinonsi),m_sequencesinonsi(sequeuncesinonsi),m_sequencesinon(sequencesinon) {
}

int NoeudInstSi::executer() {
    int executer=0;
  for(int i=0; i<m_conditionsinonsi.size();i++){ 
     if (m_conditionsi->executer() && i==0)
     {
         m_sequencesi->executer();
         executer=1;
     }
     if(m_conditionsinonsi[i]->executer()&&i<m_conditionsinonsi.size()&&executer==0){
         m_sequencesinonsi[i]->executer();
         executer=1;
     }
     if(executer!=1)
     {    
         m_sequencesinon->executer();
     } 
  }  
  return 0; // La valeur renvoyée ne représente rien !
}

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {
  while (m_condition->executer()) m_sequence->executer();
  return 0; // La valeur renvoyée ne représente rien !
}

NoeudInstPour::NoeudInstPour(Noeud* condition1,Noeud* condition2,Noeud* condition3, Noeud* sequence)
: m_condition1(condition1),m_condition2(condition2),m_condition3(condition3), m_sequence(sequence) {
}

int NoeudInstPour::executer() {
  for (m_condition1->executer();m_condition2->executer();m_condition3->executer()) m_sequence->executer();
  return 0; // La valeur renvoyée ne représente rien !
}

NoeudInstEcrire::NoeudInstEcrire(Noeud* message) 
:m_message(message){
}

int NoeudInstEcrire::executer(){
    cout << m_message;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudRepeter::NoeudRepeter(Noeud* seqInst, Noeud* condition)
: m_seqInst(seqInst), m_condition(condition){
}

int NoeudRepeter::executer(){
    do {m_seqInst->executer();} while(!m_condition->executer());
    return 0;
}