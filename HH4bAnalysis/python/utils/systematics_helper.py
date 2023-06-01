# Merge a list of regular expressions for different systematics types
# into a single regex for the SystematicsSvc
def consolidate_systematics_regex(regex_list):
    # Default to a single match-all
    if regex_list == ['.*']:
        syst_regex = '.*'
    else:
        for r in regex_list:
            invalid = ['', '.*', 'NOSYS', '^$']
            if r in invalid:
                raise ValueError(f"'{r}' in systematics regex is invalid")
        # Start with match for empty string i.e. NOSYS
        syst_regex = '(^$)|' + '|'.join([f'(^{r}.*)' for r in regex_list])
    return syst_regex
