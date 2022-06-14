#ifndef RB_TREE_RBTREE_HPP
#define RB_TREE_RBTREE_HPP

#include <algorithm>
#include "queue"

//just add some annotation

template<class V>
class Comp {
public:
    int operator()(const V &v1, const V &v2) {
        return v1 - v2;
    }

};

template<class T>
class Node;

template<class Val>
class Walker {

public:
    void onNode(Node<Val> &node) {
        std::cout << node;
    }

    void onRowEnd() {
        std::cout << std::endl;
    }
};

enum Color {
    red, black
};

template<class Val>
class Node {
public:
    typedef Node *nptr;
public:
    Val val;
    nptr parent;
    nptr left;
    nptr right;
    Color color;
public:
    Node(Val val, Color c, nptr parent, nptr left, nptr right);

    Node(const Node &node) {
        this->color = node.color;
        this->val = node.val;
        this->left = node.left;
        this->right = node.right;
        this->parent = node.parent;
    }

};

template<class Val>
std::ostream &operator<<(std::ostream &os, Node<Val> &node) {
    Node<Val> *p = node.parent;
    if (node.color == red) {
        os << " [C:" << "red  ";
    } else {
        os << " [C:" << "black";
    }

    if (p == nullptr) {
        os << ",P:" << "root";
    } else {
        os << ",P:" << p->val;
    }
    os << "," << "V:" << node.val << "] ";
    return os;
}

template<class Val>
Node<Val>::Node(Val val, Color c, Node::nptr parent, Node::nptr left, Node::nptr right) {
    this->val = val;
    this->color = c;
    this->parent = parent;
    this->left = left;
    this->right = right;
};

template<class Val,
        template<class V> class Comparator,
        template<class N> class Walker>
class RbTree {
private:
    typedef typename Node<Val>::nptr nptr;
    nptr root;
    Comparator<Val> comparator;
    Walker<Val> walker;
public:
    RbTree() {
        root = nullptr;
    }

    ~RbTree() {
        clear(this->root);
    }

public:
    void insert(const Val &val) {
        nptr node = new Node<Val>(val, red, nullptr, nullptr, nullptr);
        insert(node);
    }

    bool contains(const Val &val) {
        return find(val) != nullptr;
    }

    Val remove(const Val &val) {
        nptr node = find(val);
        if (node == nullptr) {
            return Val();
        }
        node = remove(node);
        Val ret = node->val;
        delete node;
        return ret;
    }

    void clear() {
        clear(this->root);
    }

    void traversal() {
        if (this->root == nullptr) {
            return;
        }
        std::queue<nptr> queue;
        nptr last = this->root;
        nptr lastN = this->root;
        queue.push(this->root);

        while (!queue.empty()) {
            nptr node = queue.front();
            walker.onNode(*node);
            queue.pop();
            if (node->left != nullptr) {
                queue.push(node->left);
                lastN = node->left;
            }
            if (node->right != nullptr) {
                queue.push(node->right);
                lastN = node->right;
            }
            if (node == last) {
                walker.onRowEnd();
                last = lastN;
            }
        }

    }

private:

    void leftRotate(nptr node) {
        nptr rNode = node->right;
        node->right = rNode->left;

        if (rNode->left != nullptr) {
            rNode->left->parent = node;
        }

        rNode->parent = node->parent;
        if (node->parent == nullptr) {
            this->root = rNode;
        } else {
            if (node == node->parent->left) {
                node->parent->left = rNode;
            } else {
                node->parent->right = rNode;
            }
        }

        rNode->left = node;
        node->parent = rNode;
    }

    void rightRotate(nptr node) {
        nptr lNode = node->left;
        node->left = lNode->right;

        if (lNode->right != nullptr) {
            lNode->right->parent = node;
        }

        lNode->parent = node->parent;
        if (node->parent == nullptr) {
            this->root = lNode;
        } else {
            if (node == node->parent->left) {
                node->parent->left = lNode;
            } else {
                node->parent->right = lNode;
            }
        }

        lNode->right = node;
        node->parent = lNode;
    }

    void insert(nptr node) {
        nptr current = this->root, parent = nullptr;
        int comp = 0;
        while (current != nullptr) {
            parent = current;
            comp = this->comparator(node->val, current->val);
            if (comp < 0) {
                current = current->left;
            } else if (comp > 0) {
                current = current->right;
            } else {
                current->val = node->val;
                delete node;
                return;
            }
        }

        node->parent = parent;
        if (parent == nullptr) {
            this->root = node;
        } else {
            if (comp < 0) {
                parent->left = node;
            } else {
                parent->right = node;
            }
        }

        insertFixup(node);
    }

    void insertFixup(nptr node) {
        nptr parent = nullptr, gparent = nullptr;
        //父节点存在且父节点为红色，则需要调整，因为插入的新节点最开始总为红色
        while ((parent = node->parent) != nullptr && parent->color == red) {
            gparent = parent->parent;
            //父节点是祖父节点的左子节点
            if (parent == gparent->left) {
                nptr uncle = gparent->right;
                //case 1 : 叔叔节点存在且为红色
                if (uncle != nullptr && uncle->color == red) {
                    //变色，将父节点与叔叔节点涂黑，祖父节点涂红，然后 将祖父节点作为当前节点重复上述过程
                    parent->color = black;
                    uncle->color = black;
                    gparent->color = red;
                    node = gparent;
                    continue;
                } else {//叔叔节点不存在 或 为黑色
                    if (node == parent->right) {//case 2: 当前节点是父节点的右子节点
                        leftRotate(parent);//将父节点左旋，使当前节点变为父节点，原父节点变为当前节点的左 子节点 转为 case 3
                        nptr tmp = parent;
                        parent = node;
                        node = tmp;
                    }

                    //case 3: 叔叔节点是黑色，且当前节点是父节点的左子节点
                    parent->color = black;//将父节点涂黑
                    gparent->color = red;//祖父节点涂红
                    rightRotate(gparent);//将祖父节点右旋，就会导致父节点（黑色）被提升上去，原祖父节点变为父节点的右子节点（红色），从而平衡
                }

            } else {
                nptr uncle = gparent->left;
                //case 1 : 叔叔节点存在且为红色
                if (uncle != nullptr && uncle->color == red) {
                    //变色，将父节点与叔叔节点涂黑，祖父节点涂红，然后 将祖父节点作为当前节点重复上述过程
                    parent->color = black;
                    uncle->color = black;
                    gparent->color = red;
                    node = gparent;
                    continue;
                } else {//叔叔节点不存在 或 为黑色
                    if (node == parent->left) {//case 2: 当前节点是父节点的右子节点
                        rightRotate(parent);//将父节点左旋，使当前节点变为父节点，原父节点变为当前节点的左 子节点 转为 case 3
                        nptr tmp = parent;
                        parent = node;
                        node = tmp;
                    }

                    //case 3: 叔叔节点是黑色，且当前节点是父节点的左子节点
                    parent->color = black;//将父节点涂黑
                    gparent->color = red;//祖父节点涂红
                    leftRotate(gparent);//将祖父节点右旋，就会导致父节点（黑色）被提升上去，原祖父节点变为父节点的右子节点（红色），从而平衡
                }
            }
        }
        this->root->color = black;
    }

    nptr find(const Val &val) {
        nptr node = this->root;
        while (node != nullptr) {
            int comp = this->comparator(val, node->val);
            if (comp == 0) {
                return node;
            } else if (comp < 0) {
                node = node->left;
            } else {
                node = node->right;
            }
        }
        return node;
    }

    nptr remove(nptr node) {
        if (node->left == nullptr && node->right == nullptr) {//被删除节点没有子节点，直接删除该节点
            if (node->parent == this->root) {
                this->root = nullptr;
                return node;
            }
            if (node == node->parent->left) {
                node->parent->left = nullptr;
            } else {
                node->parent->right = nullptr;
            }
            node->parent = nullptr;
            return node;
        }

        nptr target;//最终要删除的节点
        if (node->left == nullptr || node->right == nullptr) {
            //左右子节点只要一个不存在，则将node作为要删除的节点
            target = node;
        } else {
            //左右子节点都存在时，转换为删除后继节点，相当于转换为上面的情况了
            target = getSuccessor(node);
        }
        nptr replace = target->left == nullptr ? target->right : target->left;//最终要删除节点的替补节点
        replace->parent = target->parent;
        if (target->parent == nullptr) {
            this->root = replace;
        } else {
            if (target == target->parent->left) {
                target->parent->left = replace;
            } else {
                target->parent->right = replace;
            }
        }

        if (node != target) {//不相等，说明发生了节点替换，则将替换节点的val覆盖到target
            Val val = node->val;
            node->val = target->val;
            target->val = val;
        }
        if (target->color == black) {//如果删除的是黑色节点，则需要平衡
            removeFixUp(replace);
        }
        return target;
    }

    /**
     * @param node 表示待修正的节点，即后继节点的子节点（因为后继节点被挪到删除节点的位置去了）
     */
    void removeFixUp(nptr node) {
        //如果替补上来的节点是黑色才需要重新平衡，如果是红色就直接设置为黑色就平衡了
        while (node->color == black && node != this->root) {
            if (node == node->parent->left) {// node是左子节点
                nptr brother = node->parent->right;
                if (brother->color == red) {
                    /*  case 1:
                     *      兄弟节点是红色,此时parent比为黑色，只需要将brother设为黑色，parent设为红色，然后对parent左旋
                     *      左旋后对右子树 平衡性质不变，但因为brother的左子树变成了原parent的右子树，所以parent必须设为红色 才能保证
                     *      右子树的平衡，然后将brother更新
                     *      如此，便将这种情况转为了兄弟节点是黑色的情况 case 2
                     */
                    brother->color = black;
                    node->parent->color = red;
                    leftRotate(brother->parent);
                    brother = node->parent->right;
                }

                if (node->left->color == black && node->right->color == black) {
                    /**
                     * case 2: 兄弟节点是黑色，并且兄弟节点的左右节点都为黑色
                     *          这种情况需要将兄弟节点设为红色(相当于减少了兄弟子树的黑色层数)，然后将node设置为parent(将parent视为替补上来的节点)
                     */
                    brother->color = red;
                    node = node->parent;
                } else if (brother->right->color == black) {
                    /**
                     * case 3 :兄弟节点是黑色，并且兄弟节点的左子节点时 红色 ，右子节点是 黑色
                     *         将 转换为 case 4
                     */
                    brother->left->color = black;
                    brother->color = red;
                    rightRotate(brother);
                    brother = node->parent->right;
                }
                /**
                 * case 4 : 兄弟节点是黑色，兄弟节点的左子节点是任意颜色，右子节点是红色
                 */
                brother->color = brother->parent->color;
                node->parent->color = black;
                brother->right->color = black;
                leftRotate(node->parent);
                node = this->root;
            } else {
                nptr brother = node->parent->left;
                if (brother->color == red) {
                    brother->color = black;
                    node->parent->color = red;
                    rightRotate(brother->parent);
                    brother = node->parent->left;
                }
                if (node->left->color == black && node->right->color == black) {
                    brother->color = red;
                    node = node->parent;
                } else if (brother->left->color == black) {
                    brother->right->color = black;
                    brother->color = red;
                    leftRotate(brother);
                    brother = node->parent->left;
                }
                brother->color = brother->parent->color;
                node->parent->color = black;
                brother->left->color = black;
                rightRotate(node->parent);
                node = this->root;
            }
        }
        node->color = black;
    }

    /**
     * 获取后继节点
     */
    nptr getSuccessor(nptr node) {
        nptr successor = node->right;
        while (successor != nullptr) {
            successor = successor->left;
        }
        return successor;
    }

    void clear(nptr node) {
        if (node != nullptr) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }
};

#endif //RB_TREE_RBTREE_HPP
