from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ViewSelectionConfig(ConfigBlock):
    """
    Configuration for turning selection decorations into
    a filtered view container.

    For the standard usage where we take a calibrated container
    with some selection decoration and want the output to be filtered
    but retain the same names (e.g. AnalysisElectrons), just pass the
    container name and leave the rest of the options blank.

    For special cases, e.g. jets where we want different names for the
    full and filtered containers (all calibrated jets for MET, with
    pt/eta/JVT selection for other operations), we may need to
    override the input container.

    By default will apply all selections on the container. A specific
    subselection can also be applied.
    """

    def __init__(
        self,
        containerName,
        input='',
        original='',
        selection=''
    ):
        super().__init__(containerName)
        self.containerName = containerName
        self.addOption('selectionName', selection, type=str)
        self.input = input if input else containerName
        self.original = original

    def makeAlgs(self, config):
        alg = config.createAlgorithm(
            'CP::AsgViewFromSelectionAlg',
            'ViewCreator_' + self.containerName
        )
        if self.input == self.containerName:
            alg.input = config.readName(self.containerName)
            selection = [
                config.getFullSelection(self.containerName, self.selectionName)
            ]
        else:
            config.setSourceName(
                containerName=self.containerName,
                sourceName=self.input,
                originalName=self.original,
            )
            alg.input = config.readName(self.input)
            selection = [
                config.getFullSelection(self.input, self.selectionName)
            ]

        alg.output = config.copyName(self.containerName)
        alg.selection = selection


def makeViewSelectionConfig(
    seq,
    containerName,
    input='',
    original='',
    selection='',
):

    config = ViewSelectionConfig(containerName, input, original, selection)
    seq.append(config)
