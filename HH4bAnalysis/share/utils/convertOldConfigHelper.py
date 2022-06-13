# Convert old style configurable to new via CompFactory
# Services and public tools will not be handled and would need to be
# added directly to the top-level CA
def convertComp(CompFactory, myComp):
    newcomp = CompFactory.getComp(myComp.getType())(myComp.getName())
    for p, v in myComp.getProperties().items():
        if v != "<no value>":
            # Has a componentType, indicating handle
            # Could use this to flag and store any
            # services or public tools
            if hasattr(v, "componentType"):
                setattr(newcomp, p, v.toStringProperty())
            # Has a name, but not a componentType (indicating component)
            elif hasattr(v, "getName"):
                setattr(newcomp, p, convertComp(CompFactory, v))
            else:
                setattr(newcomp, p, v)
    return newcomp


# Assume flat, recursion to get all sequences is
# probably tedious but might be needed
def convertSequenceAndGetAlgs(CompFactory, mySeq):
    newseq = CompFactory.AthSequencer(mySeq.name())
    newalgs = []
    for alg in mySeq:
        newalgs.append(convertComp(CompFactory, alg))
    return newseq, newalgs
