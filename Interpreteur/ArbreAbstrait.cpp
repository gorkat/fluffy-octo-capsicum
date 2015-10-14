#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"
#include <typeinfo>

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

NoeudInstSi::NoeudInstSi(vector <Noeud*> conditionsinonsi,vector <Noeud*> sequeuncesinonsi)
:m_conditionsinonsi(conditionsinonsi),m_sequencesinonsi(sequeuncesinonsi) {
}

int NoeudInstSi::executer() {
  int executer=0;
  int i=0;
  while(executer==0){ 
     if(m_conditionsinonsi[i]->executer()&&i<m_conditionsinonsi.size()){
         m_sequencesinonsi[i]->executer();
         executer=1;
     }
     else if(i==m_conditionsinonsi.size()-1&& m_conditionsinonsi.size()<m_sequencesinonsi.size() &&executer==0)
     {    
         m_sequencesinonsi[i]->executer();
         executer=1;
     } 
     i++;
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
    if(m_condition1!=nullptr){
        m_condition1->executer();
    }
    while(m_condition2->executer()){
        m_sequence->executer();
        if(m_condition3!=nullptr){
            m_condition3->executer();
        }
    }
  return 0; // La valeur renvoyée ne représente rien !
}

NoeudInstEcrire::NoeudInstEcrire(vector <Noeud*> message) 
:m_message(message){
}

int NoeudInstEcrire::executer(){
    int i=0;
    while(i<m_message.size()){
        if(typeid(*m_message[i])==typeid(SymboleValue) &&  *((SymboleValue*)m_message[i])== "<CHAINE>" ){
            Symbole* mess = dynamic_cast<Symbole*>(m_message[i]);
            string message = mess->getChaine();
            int p ,j;
                for(j=0;message[j] != '\0';j++)
                    {
                    if (message[j] == '"')  // on a trouve
                        {
                        for(p=j;message[p]!=0;p++)
                            message[p]=message[p+1];
                        }
                    }  
            cout<< message;
        } else {
            cout<<m_message[i]->executer();
        }
        i++;
    }
    cout << endl;
    return 0;
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

NoeudInstLire::NoeudInstLire(vector <Noeud*> variable):m_variable(variable){}

int NoeudInstLire::executer(){
    int a;
    int i =0;
    while(i<m_variable.size()){
        cin >> a;
        ((SymboleValue*) m_variable[i])->setValeur(a);
        i++;
    }
}