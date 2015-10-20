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

void NoeudSeqInst::traduitenCpp(ostream & cout, unsigned int indentation) const{
    for(int i=0;i<m_instructions.size();i++){
        m_instructions[i]->traduitenCpp(cout,0);
        cout<<";"<<endl;
    }
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

void NoeudAffectation::traduitenCpp(ostream & cout, unsigned int indentation) const{
    cout<< setw(4*indentation)<<""<<((Symbole*)m_variable)->getChaine()<<"=";
//    if (typeid())
    m_expression->traduitenCpp(cout,indentation);    
    cout<<";"<<endl;
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

void NoeudOperateurBinaire::traduitenCpp(ostream & cout, unsigned int indentation) const{
    if(m_operandeDroit!=nullptr){
       cout<< setw(4*indentation)<<""<<((SymboleValue*)m_operandeGauche)->getChaine()<<m_operateur.getChaine()<<((SymboleValue*)m_operandeDroit)->getChaine()<<";"<<endl;
    }
    else{
       cout<<setw(4*indentation)<<""<<((SymboleValue*)m_operandeGauche)->getChaine()<<";"<<endl;
    }
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

void NoeudInstSi::traduitenCpp(ostream & cout, unsigned int indentation) const{
    for(int i=0;i<m_sequencesinonsi.size();i++){
        if (i==0){
          cout << setw(4*indentation) << "" << "if (";
          m_conditionsinonsi[i]->traduitenCpp(cout,0);
          cout<<") {"<<endl;
          m_sequencesinonsi[i]->traduitenCpp(cout, indentation+1);
          cout<<setw(4*indentation) <<""<<"}"<<endl;
        }
        else if (i<m_conditionsinonsi.size()){
          cout << setw(4*indentation) << "" << "else if (";
          m_conditionsinonsi[i]->traduitenCpp(cout,0);
          cout<<") {"<<endl;
          m_sequencesinonsi[i]->traduitenCpp(cout, indentation+1);
          cout<<setw(4*indentation) <<""<<"}"<<endl;
        }
        else{
          cout << setw(4*indentation) << "" << "else {";
          m_sequencesinonsi[i]->traduitenCpp(cout, indentation+1);
          cout<<setw(4*indentation) <<""<<"}"<<endl;
        }
    }   
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {
  while (m_condition->executer()) m_sequence->executer();
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstTantQue::traduitenCpp(ostream & cout, unsigned int indentation) const{
    cout << setw(4*indentation) << "" << "while (";
    m_condition->traduitenCpp(cout,0);
    cout<<") {"<<endl;
    m_sequence->traduitenCpp(cout,indentation+1);
    cout<<setw(4*indentation) <<""<<"}"<<endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

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

void NoeudInstPour::traduitenCpp(ostream & cout, unsigned int indentation) const{
    cout << setw(4*indentation) << "" << "while (";
    m_condition1->traduitenCpp(cout,0);
    cout<<";";
    m_condition2->traduitenCpp(cout,0);
    cout<<";";
    m_condition3->traduitenCpp(cout,0);
    cout<<") {"<<endl;
    m_sequence->traduitenCpp(cout,indentation+1);
    cout<<setw(4*indentation) <<""<<"}"<<endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire(vector <Noeud*> message) 
:m_message(message){
}

int NoeudInstEcrire::executer(){
    int i=0;
    while(i<m_message.size()){
        if(typeid(*m_message[i])==typeid(SymboleValue) &&  *((SymboleValue*)m_message[i])== "<CHAINE>" ){
            Symbole* mess = dynamic_cast<Symbole*>(m_message[i]);
            string message = mess->getChaine();
            message.erase(0,1);
            message.pop_back();
            cout<< message;
        } else {
            cout<<m_message[i]->executer();
        }
        i++;
    }
    cout << endl;
    return 0;
}

void NoeudInstEcrire::traduitenCpp(ostream & cout, unsigned int indentation) const{
    cout << setw(4*indentation) << "" << "cout <<";
    for(int i=0;i<m_message.size();i++){
        m_message[i]->traduitenCpp(cout,0);
        if(m_message.size()>1){
            cout<<"<<";
        }
    }
    cout<<";"<<endl;
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

void NoeudRepeter::traduitenCpp(ostream & cout, unsigned int indentation) const{
    cout << setw(4*indentation) << "" << "do {" << endl;
    m_seqInst->traduitenCpp(cout,indentation+1);
    cout<<setw(4*indentation) <<""<<"} while(!";
    m_condition->traduitenCpp(cout,0);
    cout<<");"<<endl;
    
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstLire
////////////////////////////////////////////////////////////////////////////////

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

void NoeudInstLire::traduitenCpp(ostream & cout, unsigned int indentation) const{
    for(int i=0;i<m_variable.size();i++){
        cout << setw(4*indentation) << "" << "cin >>";
        m_variable[i]->traduitenCpp(cout,0);
        cout<<");"<<endl;
    }  
}      