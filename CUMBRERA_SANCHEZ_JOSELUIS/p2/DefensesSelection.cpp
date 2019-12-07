// ###### Config options ################


// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include<cstdlib>
#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"

using namespace Asedio;

struct Defensa{
    float valor;
    unsigned int peso;
    Defense* def;
};

//Calculamos el peso y el valor de cada defensa.
std::vector<Defensa> calcularValores(std::list<Defense*> defenses){
    std::vector<Defensa> Valores;
    Defensa def;

    std::list<Defense*>::iterator it=defenses.begin();
    while(it!=defenses.end()){
        def.valor=((*it)->damage/(*it)->dispersion)*(*it)->attacksPerSecond+((*it)->range/10)+((*it)->health/200);
        def.peso=(*it)->cost;
        def.def=(*it);
        Valores.push_back(def);
        it++;
    }
    return Valores;
}

//Ordenamos defenses segun su peso.
void ordenar(std::vector<Defensa> Valores){
    Defensa temporal;

    for(int i=0;i!=Valores.size();i++){
        for(int j=0;j!=Valores.size()-1;j++){
            if(Valores[j].peso <Valores[j+1].peso){
                temporal=Valores[j];
                Valores[j]=Valores[j+1];
                Valores[j+1]=temporal;
            }
        }
    }
}

//Calcula el numero total de defensas.
int num(std::list<Defense*> defenses){
    int i=0;
    for(std::list<Defense*>::iterator it=defenses.begin();it!=defenses.end();it++){
        i++;
    }
    return i;
}

std::vector<float> losValores(std::vector<Defensa> Valores){
    std::vector<float> aux;
    for(int i=0;i<Valores.size();i++){
        aux.push_back(Valores[i].valor);
    }
    return aux;
}

std::vector<unsigned int> losPesos(std::vector<Defensa> Valores){
    std::vector<unsigned int> aux;
    for(int i=0;i<Valores.size();i++){
        aux.push_back(Valores[i].peso);
    }
    return aux;
}

std::vector<Defense*> lasDefensas(std::vector<Defensa> Valores){
    std::vector<Defense*> aux;
    for(int i=0;i<Valores.size();i++){
        aux.push_back(Valores[i].def);
    }
    return aux;
}

int maximo(int a,int b){
    if(a>b){
        return a;
    }else{
        return b;
    }
}

std::list<Defense*> vectortolist(std::vector<Defense*> defensas){
    std::list<Defense*> final;
    
    for(int i=0;i<defensas.size();i++){
        final.push_back(defensas[i]);
    }
    return final;
}

std::list<Defense*> esMochila(std::vector<float> valor,std::vector<unsigned int> peso,std::vector<Defense*> defensas,
               unsigned int ases,int num){
    int matriz[num][ases+1];

    for(int c=0;c<=ases;c++){
        if(c<peso[0]){
            matriz[0][c]=0;
        }else{
            matriz[0][c]=valor[0];
        }
    }

    for(int j=1;j<num;j++){
        for(int c=0;c<=ases;c++){
            if(c<peso[j]){
                matriz[j][c]=matriz[j-1][c];
            }else{
                if(matriz[j-1][c]>=(matriz[j-1][c-peso[j]]+valor[j])){
                    matriz[j][c]=matriz[j-1][c];
                }else{
                    matriz[j][c]=matriz[j-1][c-peso[j]]+valor[j];
                }
            }
        }
    }
    
    std::vector<Defense*> defOrdenadas;

    int fil=num-1;
    int col=ases;

    while(fil!=0 && col!=0){
        if(fil==1 && matriz[fil][col]==matriz[fil][col]){
            defOrdenadas.push_back(defensas[fil]);
            col--;
            fil--;
        }else{
            if(matriz[fil][col]!=matriz[fil-1][col]){
                defOrdenadas.push_back(defensas[fil]);
                col=col-peso[fil];
                fil--;
            }else{
                col--;
                fil--;
            }
        }
    }

    return vectortolist(defOrdenadas);
}

//Algoritmo de la mochila.
std::list<Defense*> mochila(unsigned int ases,std::vector<Defensa> Valores,int num){
    std::vector<float> valor=losValores(Valores);
    std::vector<unsigned int> peso=losPesos(Valores);
    std::vector<Defense*> defensas=lasDefensas(Valores);

    return esMochila(valor,peso,defensas,ases,num);
}

void DEF_LIB_EXPORTED selectDefenses(std::list<Defense*> defenses,unsigned int ases,std::list<int> &selectedIDs
            ,float mapWidth,float mapHeight,std::list<Object*> obstacles){
    //Guardamos la base.
    Defense* Base=(*defenses.begin());
    defenses.erase(defenses.begin());
    //Calculamos el valor de cada defensa.
    std::vector<Defensa> Valores=calcularValores(defenses);
    //Ordenamos Valores y defensas en funcion de sus pesos.
    ordenar(Valores);
    //Realizamos TSP.
    //Restamos al dinero que poseemos el coste de la base.
    ases=ases-Base->cost;
    defenses=mochila(ases,Valores,num(defenses));

    defenses.push_front(Base);

    std::list<Defense*>::iterator it=defenses.begin();
    unsigned int cost=0;
    while(it!=defenses.end()){
        if(cost+(*it)->cost<=ases) {
            selectedIDs.push_back((*it)->id);
            cost+=(*it)->cost;
        }
        ++it;
    }
}
