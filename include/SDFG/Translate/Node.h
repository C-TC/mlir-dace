#ifndef SDFG_Translation_Node_H
#define SDFG_Translation_Node_H

#include "SDFG/Dialect/Dialect.h"
#include "SDFG/Translate/JsonEmitter.h"
#include "mlir/IR/Location.h"

namespace mlir::sdfg::translation {
class Emittable;

class Attribute;
class Connector;
class Assignment;

class Node;
class SDFG;
class State;
class StateImpl;

class InterstateEdge;
class MultiEdge;

class ConnectorNode;
class Access;
class Tasklet;

//===----------------------------------------------------------------------===//
// Interfaces
//===----------------------------------------------------------------------===//

class Emittable {
public:
  virtual void emit(emitter::JsonEmitter &jemit) = 0;
};

//===----------------------------------------------------------------------===//
// DataClasses
//===----------------------------------------------------------------------===//

class Attribute {
public:
  std::string name;
  // Store attribute or string?
};

class Connector {
public:
  ConnectorNode *parent;
  std::string name;
};

class Assignment {
public:
  std::string key;
  std::string value;
};

//===----------------------------------------------------------------------===//
// Node
//===----------------------------------------------------------------------===//

class Node {
protected:
  unsigned id;
  Location location;
  std::string label;
  std::vector<Attribute> attributes;
  Node *parent;

  Node(Location location) : id(0), location(location) {}

public:
  void setID(unsigned id) { this->id = id; }
  unsigned getID() { return id; }

  Location getLocation() { return location; }

  void setLabel(StringRef label) { this->label = label.str(); }
  StringRef getLabel() { return label; }

  void setParent(Node *parent) { this->parent = parent; }
  Node *getParent() { return parent; }

  // check for existing attribtues
  // Replace or add to list
  void addAttribute(Attribute attribute);
};

//===----------------------------------------------------------------------===//
// SDFG
//===----------------------------------------------------------------------===//

class SDFG : public Node, public Emittable {
private:
  std::map<unsigned, State> lut;
  std::vector<State> nodes;
  std::vector<InterstateEdge> edges;

public:
  SDFG(Location location) : Node(location) {}

  State lookup(unsigned id);
  void addState(unsigned id, State state);
  void addEdge(InterstateEdge edge);

  void emit(emitter::JsonEmitter &jemit) override;
};

//===----------------------------------------------------------------------===//
// State
//===----------------------------------------------------------------------===//

class StateImpl : public Node, public Emittable {

private:
  std::map<unsigned, ConnectorNode *> lut;
  std::vector<ConnectorNode> nodes;
  std::vector<MultiEdge> edges;

public:
  StateImpl(Location location) : Node(location) {}

  void addNode(ConnectorNode node);
  void addEdge(MultiEdge edge);

  void emit(emitter::JsonEmitter &jemit) override;
};

class State : public Emittable {
private:
  std::shared_ptr<StateImpl> ptr;

public:
  State(Location location) : ptr(std::make_shared<StateImpl>(location)){};

  StateImpl *operator->() const { return ptr.get(); }
  std::shared_ptr<StateImpl> operator*() const { return ptr; }

  void emit(emitter::JsonEmitter &jemit) override { ptr->emit(jemit); }
};

//===----------------------------------------------------------------------===//
// Edges
//===----------------------------------------------------------------------===//

class InterstateEdge : public Emittable {
protected:
  State source;
  State destination;

  std::string condition;
  std::vector<Assignment> assignments;

public:
  InterstateEdge(State source, State destination)
      : source(source), destination(destination) {}

  // Warn on override
  void setCondition(StringRef condition);
  // Check for duplicates
  void addAssignment(Assignment assignment);

  void emit(emitter::JsonEmitter &jemit) override;
};

class MultiEdge : public Emittable {

private:
  Connector *source;
  Connector *destination;
  mlir::sdfg::SizedType shape;

public:
  MultiEdge(Connector *source, Connector *destination);

  void emit(emitter::JsonEmitter &jemit) override;
};

//===----------------------------------------------------------------------===//
// ConnectorNode
//===----------------------------------------------------------------------===//

class ConnectorNode : public Node {

protected:
  std::vector<Connector> connectors;
};

class Tasklet : public ConnectorNode {};
class Access : public ConnectorNode {};

/* class MapBegin : public ConnectorNode {};
class MapEnd : public ConnectorNode {};

class ConsumeBegin : public ConnectorNode {};
class ConsumeEnd : public ConnectorNode {}; */

} // namespace mlir::sdfg::translation

#endif // SDFG_Translation_Node_H
