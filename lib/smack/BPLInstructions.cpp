//
// This file is distributed under the MIT License. See LICENSE for details.
//
#include "BPLInstructions.h"

using namespace smack;

BPLExpr* BPLAssignInst::getLeft() const {
  return left;
}

BPLExpr* BPLAssignInst::getRight() const {
  return right;
}

void BPLAssignInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  os << *left << " := " << *right << ";\n";
}

const Function* CalledFunction::getCalledFunction() {
  return calledFunction;
}

BPLCallInst::~BPLCallInst() {
  std::for_each(calledFunctions.begin(), calledFunctions.end(), &CalledFunction::destroy);
  calledFunctions.clear();
}

CalledFunction* BPLCallInst::addCalledFunction(const Function* func) {
  CalledFunction* calledFunction = new CalledFunction(func);
  calledFunctions.push_back(calledFunction);
  return calledFunction;
}

void BPLCallInst::setReturnVar(BPLVarExpr* returnVarP) {
  returnVar = returnVarP;
}

void BPLCallInst::addParam(BPLExpr* param) {
  params.push_back(param);
}

void BPLCallInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  assert(isa<CallInst>(inst) && "Call instruction has to be CallInst");
  
  if (calledFunctions.empty()) {
    // TODO: this is unsound!
    os << "// Empty call\n";
  } else if (calledFunctions.size() == 1) {
    CalledFunction* calledFunc = calledFunctions[0];

    const Function* func = calledFunc->getCalledFunction();
    os << "call ";
    if (returnVar != NULL) {
      os << *returnVar << " := ";
    }
    os << func->getName().str();
    os << "(";
    for(std::vector<BPLExpr*>::const_iterator
        j = params.begin(), bj = params.begin(), ej = params.end(); j != ej; ++j) {
      if (j != bj) {
        os << ", ";
      }
      os << *j;
    }
    os << ");\n";

  } else {
    unsigned long uniqueLabelSufix = (unsigned long)this;

    os << "goto ";
    int labelCounter = 0;
    for (std::vector<CalledFunction*>::const_iterator i = calledFunctions.begin(),
        b = calledFunctions.begin(), e = calledFunctions.end(); i != e; ++i, ++labelCounter) {
      if (i != b) {
        os << ", ";
      }
      os << "label_call_" << uniqueLabelSufix << "_" << labelCounter;
    }
    os << ";\n";

    labelCounter = 0;
    for (std::vector<CalledFunction*>::const_iterator i = calledFunctions.begin(),
        e = calledFunctions.end(); i != e; ++i, ++labelCounter) {
      os << "label_call_" << uniqueLabelSufix << "_" << labelCounter << ":\n";
      os << "  call ";
      if (returnVar != NULL) {
        os << *returnVar << " := ";
      }
      const Function* func = (*i)->getCalledFunction();
      os << func->getName().str();
      os << "(";
      for(std::vector<BPLExpr*>::const_iterator
          j = params.begin(), bj = params.begin(), ej = params.end(); j != ej; ++j) {
        if (j != bj) {
          os << ", ";
        }
        os << *j;
      }
      os << ");\n";

      os << "  goto label_call_" << uniqueLabelSufix << "_end;\n";
    }
    os << "label_call_" << uniqueLabelSufix << "_end:\n";
  }
}

void BPLCmpInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  assert(inst->hasName() && "Cmp instruction has to have a name");
  assert(isa<ICmpInst>(inst) && "Compare instruction has to be integer compare");
  const ICmpInst* icmp = cast<ICmpInst>(inst);
  os << "call " << translateName(icmp->getName().str()) << " := ";
  switch (icmp->getPredicate()) {
  case ICmpInst::ICMP_EQ:
    os << "__SMACK_Proc_ICMP_EQ";
    break;
  case ICmpInst::ICMP_NE:
    os << "__SMACK_Proc_ICMP_NE";
    break;
  case ICmpInst::ICMP_SGE:
    os << "__SMACK_Proc_ICMP_SGE";
    break;
  case ICmpInst::ICMP_UGE:
    os << "__SMACK_Proc_ICMP_UGE";
    break;
  case ICmpInst::ICMP_SLE:
    os << "__SMACK_Proc_ICMP_SLE";
    break;
  case ICmpInst::ICMP_ULE:
    os << "__SMACK_Proc_ICMP_ULE";
    break;
  case ICmpInst::ICMP_SLT:
    os << "__SMACK_Proc_ICMP_SLT";
    break;
  case ICmpInst::ICMP_ULT:
    os << "__SMACK_Proc_ICMP_ULT";
    break;
  case ICmpInst::ICMP_SGT:
    os << "__SMACK_Proc_ICMP_SGT";
    break;
  case ICmpInst::ICMP_UGT:
    os << "__SMACK_Proc_ICMP_UGT";
    break;
  default:
    assert(false && "Predicate not supported");
  }
  os << "(" << *left << ", " << *right << ");\n";
}

void BPLBoolToIntInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  assert(inst->hasName() && "BoolToInt instruction has to have a name");
  os << "call " << translateName(inst->getName().str()) << " := ";
  os << "__SMACK_BoolToInt";
  os << "(" << *boolExpr << ");\n";
}

void BPLTruncInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  assert(inst->hasName() && "Trunc instruction has to have a name");
  os << "call " << translateName(inst->getName().str()) << " := ";
  os << "__SMACK_Trunc";
  os << "(" << *operand << ");\n";
}

void BPLBinaryOperatorInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  assert(inst->hasName() && "BinaryOperator instruction has to have a name");
  assert(isa<BinaryOperator>(inst) && "Binary operator instruction has to be BinaryOperator");
  os << "call " << translateName(inst->getName().str()) << " := ";
  switch (inst->getOpcode()) {
  case Instruction::Add:
    os << "__SMACK_Add";
    break;
  case Instruction::Sub:
    os << "__SMACK_Sub";
    break;
  case Instruction::Mul:
    os << "__SMACK_Mul";
    break;
  case Instruction::SDiv:
    os << "__SMACK_SDiv";
    break;
  case Instruction::UDiv:
    os << "__SMACK_UDiv";
    break;
  case Instruction::SRem:
    os << "__SMACK_SRem";
    break;
  case Instruction::URem:
    os << "__SMACK_URem";
    break;
  case Instruction::And:
    os << "__SMACK_And";
    break;
  case Instruction::Or:
    os << "__SMACK_Or";
    break;
  case Instruction::Xor:
    os << "__SMACK_Xor";
    break;
  case Instruction::LShr:
    os << "__SMACK_LShr";
    break;
  case Instruction::AShr:
    os << "__SMACK_AShr";
    break;
  case Instruction::Shl:
    os << "__SMACK_Shl";
    break;
  default:
    assert(false && "Predicate not supported");
  }
  os << "(" << *left << ", " << *right << ");\n";
}

void BPLAllocaInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  assert(inst->hasName() && "Alloca instruction has to have a name");
  std::string varName = translateName(inst->getName().str());
  
  os << "call " << varName << " := ";
  os << "__SMACK_alloca(" << typeSize->getOffComponent() << " * " << arraySize->getOffComponent() << ");\n";
}

void BPLMallocInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  assert(inst->hasName() && "Malloc instruction has to have a name");
  std::string varName = translateName(inst->getName().str());
  
  os << "call " << varName << " := ";
  os << "__SMACK_malloc(" << arraySize->getOffComponent() << ");\n";
}

void BPLFreeInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  os << "call __SMACK_free(" << *freedPtr << ");\n";
}

void BPLAssertInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  os << "assert(" << *assertion << " != Ptr(null, 0));\n";
}

void BPLAssumeInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  os << "assume(" << *assumption << " != Ptr(null, 0));\n";
}

void BPLReturnInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  if (returnValue != NULL) {
    assert(returnVar != NULL && "Return variable shoudn't be NULL");
    os << *returnVar << " := " << *returnValue << ";\n  ";
  }
  os << "return;\n";
}

void BPLSelectInst::print(std::ostream &os) const {
  BPLInstruction::print(os);
  assert(inst->hasName() && "Select instruction has to have a name");
  os << "if (" << *condition << ") {\n";
  os << "    " << translateName(inst->getName().str()) << " := " << *trueExpr << ";\n";
  os << "  } else {\n";
  os << "    " << translateName(inst->getName().str()) << " := " << *falseExpr << ";\n";
  os << "  }\n";
}
