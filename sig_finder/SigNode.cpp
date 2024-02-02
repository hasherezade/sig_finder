/* 
* Copyright (c) 2013 hasherezade
*/

#include "SigNode.h"
using namespace sig_ma;

//--------------------------------------

bool SigNode::sig_compare::operator() (const SigNode* el1, const SigNode* el2 ) const
{
	return (*el1) < (*el2); 
}

//---

SigNode::SigNode(uint8_t _val, sig_type _vtype, uint8_t _vmask)
{
	this->v = _val;
	this->vtype = _vtype;
	this->vmask = _vmask;
}


void SigNode::_clearNodesSet(std::set<SigNode*, sig_compare> &nodesSet)
{
	for (auto itr = nodesSet.begin(); itr != nodesSet.end();) {
		SigNode* node = *itr;
		++itr;
		delete node;
	}
	nodesSet.clear();
}

SigNode::~SigNode()
{
	_clearNodesSet(immediates);
	_clearNodesSet(partials);
	_clearNodesSet(wildcards);
}

SigNode* SigNode::getWildc() const
{
	/* TODO: value masking */
	SigNode srchd(WILD_ONE, WILDC);
	std::set<SigNode*, sig_compare>::iterator found = wildcards.find(&srchd);
	if (found == wildcards.end()) return NULL;
	return (*found);
}

SigNode* SigNode::getChild(uint8_t val) const
{
	SigNode srchd(val, IMM);
	std::set<SigNode*, sig_compare>::iterator found = immediates.find(&srchd);
	if (found == immediates.end()) return NULL;
	return (*found);
}

SigNode* SigNode::getPartial(uint8_t val) const
{
	SigNode srchd(val, PARTIAL);
	std::set<SigNode*, sig_compare>::iterator found = partials.find(&srchd);
	if (found == partials.end()) return NULL;
	return (*found);
}

//-----------------------------------------


SigNode* SigNode::_insertSigNode(std::set<SigNode*, SigNode::sig_compare> &nodesSet, uint8_t val, uint8_t vmask, sig_type vtype)
{
	SigNode* f_node = NULL;
	SigNode* srchd = new SigNode(val, vtype, vmask);
	std::set<SigNode*, SigNode::sig_compare>::iterator found = nodesSet.find(srchd);
	if (found == nodesSet.end()) {
		f_node = srchd;
		nodesSet.insert(f_node);
	} else {
		f_node = *found;
		delete srchd; //already exists, no need for the new one
	}
	return f_node;
}

SigNode* SigNode::putChild(uint8_t val, uint8_t vmask)
{
	SigNode* f_node = NULL;
	sig_type vtype = (vmask == 0xFF) ? IMM : PARTIAL;
	if (vtype == IMM) {
		f_node = _insertSigNode(this->immediates, val, vmask, vtype);
	} else {
		f_node = _insertSigNode(this->partials, val, vmask, vtype);
	}
	return f_node;
}

SigNode* SigNode::putWildcard(uint8_t val)
{
	return _insertSigNode(this->wildcards, val, 0xFF, WILDC);
}
