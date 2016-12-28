#include "network.h"
#include "node.h"
#include <iostream>
#include <stdlib.h>
#include <queue>

using namespace std;

Network::Network(bool d, bool e, bool r, int n) {
    this->debug = d;
    this->extend = e;
    this->node_max = n;
    this->restruct = r;
    this->cnt.s = 0;
}

void Network::init() {

    node_list = new Node[this->node_max];
    if(node_list == NULL) return;

    // root を追加
    Node root;
    root.setId(0);
    root.setConnectionType(0);
    root.setMobile(false);
    root.setConnect(true);
    node_list[0] = root;

    // 全ノード追加
    for(int i=1; i<this->node_max; i++) {
        Node node;
        node.setId(i);
        node_list[i] = node;
    }
    cout << "\nInitializing has done!\n" << endl;
};

void Network::buildTree() {
    for(int i=1; i<this->node_max; i++) {
        Network::entryTree(&(node_list[i]));
    }

    cout << "\nTree Building has done!\n" << endl;
}

void Network::entryTree(Node* v) {
    queue<Node*> queue;

    // root ノードを queue に追加
    Node* root = &(node_list[0]);
    queue.push(root);

    // 幅優先探索で参加先を決定
    while( !queue.empty() ) {
        Node* p = queue.front();
        queue.pop();
        for(int i=0; i<CHILDREN_MAX; i++) {
            // 子ノードの接続先がある場合
            if(p->children[i] == NULL) {
                // 接続可能な相性な場合
                if(canConnect(v, p)) {
                    p->children[i] = v;
                    v->parent = p;
                    v->setLocate(i);
                    v->setConnect(true);
                    if(this->debug) printf("I'm %5d.     My Type is %d. And mobile is %d    My parent's ID is %5d.     The location is %d.\n", v->getId(), v->getConnectionType(), v->getMobile(), p->getId(), i);
                    return;
                }
            } else {
                queue.push(p->children[i]);
            }
        }
    }

    // 接続先が無かった場合、木の再構築を行い参加を試みる
    if(this->restruct) snatchMobileLocate(v, &(node_list[0]));

    if(this->debug) if(!v->getConnect()) printf("%d failed to join.\n", v->getId());
}

/**
*canConnect
*c: child  子となるノード
*p: parent 親となるノード
*/

bool Network::canConnect(Node* c, Node* p) {
    bool ret = false;

    if(p->getMobile()) ret = false;
    else if(c->getConnectionType() <= 1 || p->getConnectionType() <= 1) ret = true;
    else if(c->getConnectionType() <= 3 && p->getConnectionType() <= 3 && this->extend) ret = true;

    return ret;
}

void Network::snatchMobileLocate(Node* v, Node* d) {
    // 再帰処理の為、見つかっていた場合ここで処理終了
    if(v->getConnect()) return;

    // 参加者が Mobile 又は、交換先が不参加なら検索終了
    if(v->getMobile() || !d->getConnect()) return;

    // 交換先が Mobile 且つ、交換先の親ノードと接続可能
    if(d->getMobile() &&  canConnect(v, d->parent)) {
        if(this->debug) printf("Detect Snatch   >>> v:%5d     d:%5d\n", v->getId(), d->getId());
        Node* p = d->parent;

        p->children[d->getLocate()] = v;
        v->parent = p;
        // for(int i=0; i<CHILDREN_MAX; i++) v->children[i] = d->children[i];
        v->setLocate(d->getLocate());
        v->setConnect(true);
        d->setConnect(false);
        this->cnt.s++;

        // 追加先再検索
        entryTree(d);
    } else {
        for(int i=0; i<CHILDREN_MAX; i++) {
            if(d->children[i] != NULL) {
                snatchMobileLocate(v, d->children[i]);
            }
        }
    }
}

void Network::showResult() {
    int cnt = 0;

    for(int i=0; i<this->node_max; i++) {
        if(this->debug) {
            if(node_list[i].parent != NULL ) printf("I'm %5d.     My type is %d. And mobile is %d    My parent is %5d\n", node_list[i].getId(), node_list[i].getConnectionType(), node_list[i].getMobile(), node_list[i].parent->getId());
            else  printf("I'm %5d.     My type is %d. And mobile is %d     I have no parent.\n", node_list[i].getId(), node_list[i].getConnectionType(), node_list[i].getMobile());
        }
        if(node_list[i].getConnect()) cnt++;
    }

    free(node_list);
    if(this->restruct) cout << "\nRESTRUCTION >>>  S:" << this->cnt.s << endl;
    cout << cnt << " of " << this->node_max << " nodes joined in the Tree." << endl;
    if(cnt == this->node_max) cout << "perfect !!!111" << endl;
}
