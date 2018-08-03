#include <stdlib.h>
#include <map>
#include <vector>
#include <stdio.h>
#include <iostream>

using namespace std;

const static int INVALID_EVENT = -1;
const static int INVALID_STATE_ID = -1;
class IState{
public:
	virtual int Start(int event) = 0;
	virtual void Response() = 0;
	virtual int ID() = 0;
	virtual ~IState(){}
	virtual bool Finish() = 0;
};

class StateBase;

class StateManagerBase{
public:
	StateManagerBase(){
		m_nCurState = INVALID_STATE_ID;
	}

	int SetStartState(int stateid){
		m_nCurState = stateid;
	}

	int AddState(const int id, IState* state){
		if(m_mapState.find(id) != m_mapState.end()){
			return -1;
		}
		m_mapState[id] = state;
		return 0;
	}

	int AddTransfer(int iBeforeState, int event, int iAfterState){
		std::pair<int,int> data = std::make_pair(iBeforeState, event);
		m_mapTransfer.insert(std::make_pair(data, iAfterState));
		return 0;
	}

	//针对某些state需要异步响应的情况
	int OnStateAsyncReponse(int cbid){
		if(cbid == 0){
			return -1;
		}
		IState* state = (IState*)cbid;
		state->Response();
	}
	
	//start state
	int TransferState(int eve){
		if(m_nCurState == INVALID_STATE_ID){
			return -1;
		}
		std::map<int, IState*>::iterator itt = m_mapState.find(m_nCurState);
		if(itt != m_mapState.end()){
			IState* state = itt->second;
			return state->Start(eve);
		}else{
			return -1;
		}
		return 0;
	}

	~StateManagerBase(){
		for(std::map<int, IState*>::iterator it = m_mapState.begin(); it != m_mapState.end(); ){
			IState* state = it->second;	
			++it;
			delete state;
		}
	}
private:
	//change state
	int ChangeState(int eve){
		if(!CheckCurCanChange()){
			return -1;
		}
		IState* state = NULL;
		bool bNext = GetNextState(m_nCurState, eve, state);
		if(bNext && state){
			SetCurState(state->ID());	
		}
		return 0;
	}

	bool GetNextState(int curState,int eve, IState *state){
		state = NULL;
		std::map<std::pair<int, int>, int>::iterator it = m_mapTransfer.find(std::make_pair(curState,eve));
		if(it->second){
			int stateid = it->second;
			std::map<int, IState*>::iterator itt = m_mapState.find(stateid);
			if(itt != m_mapState.end()){
				state = itt->second;
				return true;
			}else{
				return false;
			}
		}
		return false;
	}

	int SetCurState(int state){
		m_nCurState = state;
		return 0;
	}
	
	bool CheckCurCanChange(){
		std::map<int, IState*>::iterator itt = m_mapState.find(m_nCurState);
		if(itt != m_mapState.end()){
			IState* state = itt->second;
			return state->Finish();
		}else{
			return false;
		}
		return false;
	}
private:
	std::map<int, IState*> m_mapState;
	std::map<std::pair<int, int>, int> m_mapTransfer;// < <state,eve>, state>
	int m_nCurState;
	friend class StateBase;
};

class StateBase:public IState{
public:
	virtual int OnStart(int event){
		cout << "OnStart:" << event <<" " <<m_id << endl;
		return 0;
	}
	virtual int OnResponse(){
		cout << "OnResponse:" << m_nInProcEvent << " " << m_id << endl;
		return 0;
	}

	virtual int OnError(){
		return 0;
	}
public:
	StateBase(StateManagerBase* manager, int id, bool bSync = false){
		m_pManager = manager;
		m_nInProcEvent = INVALID_EVENT;
		m_id = id;
		m_bSync = bSync;
		m_bFinish = false;
	}

	int Start(int event){
		m_nInProcEvent = event;
		if(OnStart(event)!=0){
			OnError();
			return -1;
		}
		//如果是同步的方式执行，直接走到回调
		//异步需要外部接口拉起
		if(m_bSync){
			Response();
		}
		return 0;
	}
	
	void Response(){
		m_bFinish = true;
		if(0 != OnResponse()){
			OnError();
			return;
		}
		m_pManager->ChangeState(m_nInProcEvent);	
	}

	int ID(){
		return m_id;
	}

	bool Finish(){
		return m_bFinish;
	}

	~StateBase(){
	}


private:
	StateManagerBase* m_pManager;	
	int m_nInProcEvent;
	int m_id;
	bool m_bSync;
	bool m_bFinish;
};

int main(){
	StateManagerBase *manager = new StateManagerBase();
	StateBase* state0 = new StateBase(manager,0,true);
	StateBase* state1 = new StateBase(manager,1,true);
	StateBase* state2 = new StateBase(manager,2,true);
	StateBase* state3 = new StateBase(manager,3,true);
	manager->AddState(0,state0);
	manager->AddState(1,state1);
	manager->AddState(2,state2);
	manager->AddState(3,state3);
	manager->AddTransfer(0,1,1);
	manager->AddTransfer(1,2,2);
	manager->AddTransfer(2,3,3);

	manager->SetStartState(0);

	manager->TransferState(1);
	manager->TransferState(2);
	manager->TransferState(3);
}
