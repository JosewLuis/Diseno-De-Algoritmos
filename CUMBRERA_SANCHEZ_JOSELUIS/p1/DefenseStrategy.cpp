// ###### Config options ################
//#define PRINT_DEFENSE_STRATEGY 1    // generate map images
// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include"../simulador/Asedio.h"
#include"../simulador/Defense.h"
#include<cstdlib>
#include<cstdio>
#ifdef PRINT_DEFENSE_STRATEGY
#include"ppm.h"
#endif

#ifdef CUSTOM_RAND_GENERATOR
RAND_TYPE SimpleRandomGenerator::a;
#endif

using namespace Asedio;

struct Celda{
	float x_,y_;
	float valor_;
	Celda(int x=0,int y=0){
		this->x_=x;
		this->y_=y;
	}
	Celda(const Celda& C){
		this->x_=C.x_;
		this->y_=C.y_;
		this->valor_=C.valor_;
	}
};

//Convierte el centro de una ficha en su posicion.
Vector3 cellCenterToPosition(int i,int j,float cellWidth,float cellHeight){
	return Vector3((j*cellWidth)+cellWidth*0.5f,(i*cellHeight)+cellHeight*0.5f,0);
}

//Devuelve la celda a la que corresponde una posición en el mapa
//&i_out &j_out (i,j) correspondientes al centro de la celda.
void positionToCell(const Vector3 pos,int &i_out,int &j_out,float cellWidth,float cellHeight){
	i_out=(int)(pos.y*1.0f/cellHeight); j_out=(int)(pos.x*1.0f/cellWidth);
}

//Value para la base.
float cellValueBase(int i,int j,int nCellsWidth,int nCellsHeight,std::list<Object*> obstacles){
	float distance=abs(nCellsWidth/2-i)+abs(nCellsHeight/2-j);
	std::list<Object*>::iterator it;

	for(it=obstacles.begin();it!=obstacles.end();it++){
		distance=distance+abs(i-(*it)->position.x)+abs(j-(*it)->position.y);
	}
	return distance;
}

//Valor para el resto.
float cellValue(int i,int j,Defense* Base,std::list<Object*> obstacles){
	float distance=abs(Base->position.x-i)+abs(Base->position.y-j);
	std::list<Object*>::iterator it;

	for(it=obstacles.begin();it!=obstacles.end();it++){
		distance=distance+abs(i-(*it)->position.x)+abs(j-(*it)->position.y);
	}
	return distance;
}

//Ordena el vector de Celdas en funcion de su this.valor_.
//Nos quedamos con el valor de Ordenadas.
std::list<Celda> ordenar(Celda Values[],int nCellsWidth,int nCellsHeight){
	Celda aux;
	int menor,position;
	std::list<Celda> Ordenadas;
	for(int i=0;i<nCellsWidth*nCellsHeight;i++){
		menor=Values[i].valor_;
		position=i;
		for(int j=i;j<nCellsWidth*nCellsHeight;j++){
			if(Values[j].valor_>menor){
				menor=Values[j].valor_;
				position=j;
			}
		}
		aux=Values[i];
		Values[i]=Values[position];
		Values[position]=aux;
	}

	for(int i=0;i<nCellsWidth*nCellsHeight;i++){
		Ordenadas.push_back(Values[i]);
	}
	return Ordenadas;
}

bool factibilidad(Defense* currentDefense,std::list<Defense*> defenses,std::list<Object*> obstacles,float mapWidth,float mapHeight,bool** freeCells,int i,int j){
	//Si la celda esta ya ocupada:
	if(freeCells[i][j]==false){
		return false;
	}

	//Hay varias opciones de que el resultado sea negativo, que choque con un obstaculo o que se salga de rango.
	//Sale de rango (0 o max posible):
	if(currentDefense->position.x-currentDefense->radio<=0 || currentDefense->position.x+currentDefense->radio>=mapHeight || currentDefense->position.y-currentDefense->radio<=0 || currentDefense->position.y+currentDefense->radio>=mapWidth){
		return false;
	}

	//Choca contra otra estructura del juego ya colocada (obstaculo u otra defensa):
	std::list<Defense*>::iterator verify;
	for(verify=defenses.begin();verify!=defenses.end();verify++){
		float euclid=_distance((*verify)->position,currentDefense->position);

		float radio=(*verify)->radio+currentDefense->radio;

		if(radio>=euclid){
			return false;
		}
	}

	std::list<Object*>::iterator v;
	for(v=obstacles.begin();v!=obstacles.end();v++){
		float euclid=_distance((*v)->position,currentDefense->position);

		float radio=(*v)->radio+currentDefense->radio;

		if(radio>=euclid){
			return false;
		}
	}
	
	//Si se llega hasta aqui enhorabuena, es factible.
	return true;
}

void DEF_LIB_EXPORTED placeDefenses(bool** freeCells,int nCellsWidth,int nCellsHeight,float mapWidth,float mapHeight,std::list<Object*> obstacles,std::list<Defense*> defenses){
    float cellWidth=mapWidth/nCellsWidth;
    float cellHeight=mapHeight/nCellsHeight;

	Celda Values[nCellsWidth*nCellsHeight];
	std::list<Celda> Ordenadas;

	int aux=0;
	int p,q;

	for(int i=0;i<nCellsWidth;i++){
		for(int j=0;j<nCellsHeight;j++){
			Vector3 x=cellCenterToPosition(i,j,cellWidth,cellHeight);
			positionToCell(x,p,q,cellWidth,cellHeight);
			Values[aux].x_=x.x;
			Values[aux].y_=x.y;
			Values[aux].valor_=cellValueBase(i,j,nCellsWidth,nCellsHeight,obstacles);
			aux++;
		}
	}

	Ordenadas=ordenar(Values,nCellsWidth,nCellsHeight);

	std::list<Defense*> colocadas;
    List<Defense*>::iterator currentDefense=defenses.begin();
	bool find=false;
	std::list<Celda>::iterator it;
	Defense* Base;


	for(it=Ordenadas.begin();it!=Ordenadas.end() && find==false;it++){
			(*currentDefense)->position.x=(*it).x_;
			(*currentDefense)->position.y=(*it).y_;
			positionToCell((*currentDefense)->position,p,q,cellWidth,cellHeight);
			if(factibilidad((*currentDefense),colocadas,obstacles,mapWidth,mapHeight,freeCells,p,q)){
				find=true;
				Ordenadas.erase(it);
				colocadas.push_back((*currentDefense));
				Base=(*currentDefense);
				currentDefense++;
				freeCells[p][q]=false;
			}
	}

	aux=0;

	for(int i=0;i<nCellsWidth;i++){
		for(int j=0;j<nCellsHeight;j++){
			Vector3 x=cellCenterToPosition(i,j,cellWidth,cellHeight);
			positionToCell(x,p,q,cellWidth,cellHeight);
			Values[aux].x_=x.x;
			Values[aux].y_=x.y;
			Values[aux].valor_=cellValue(i,j,Base,obstacles);
			aux++;
		}
	}

	Ordenadas=ordenar(Values,nCellsWidth,nCellsHeight);

	while(currentDefense!=defenses.end()){
		find=false;
		for(it=Ordenadas.begin();it!=Ordenadas.end() && find==false;it++){
			(*currentDefense)->position.x=(*it).x_;
			(*currentDefense)->position.y=(*it).y_;
			positionToCell((*currentDefense)->position,p,q,cellWidth,cellHeight);
			if(factibilidad((*currentDefense),colocadas,obstacles,mapWidth,mapHeight,freeCells,p,q)){
				find=true;
				Ordenadas.erase(it);
				colocadas.push_back((*currentDefense));
				currentDefense++;
				freeCells[p][q]=false;
			}
		}
	}

#ifdef PRINT_DEFENSE_STRATEGY

    float** cellValues=new float* [nCellsHeight];
    for(int i=0;i<nCellsHeight;++i){
       cellValues[i]=new float[nCellsWidth];
       for(int j=0;j<nCellsWidth;++j){
           cellValues[i][j]=((int)(cellValue(i,j)))%256;
       }
    }
    dPrintMap("strategy.ppm",nCellsHeight,nCellsWidth,cellHeight,cellWidth,freeCells,cellValues,std::list<Defense*>(),true);

    for(int i=0;i<nCellsHeight;++i)
        delete[] cellValues[i];
	delete[] cellValues;
	cellValues=NULL;

#endif
}
