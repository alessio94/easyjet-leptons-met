# Temporary hack, we should do this in a more systematic way
# The config sequence will deal with the systematics suffix
def drop_sys(name):
    return name.replace('_%SYS%', '')
