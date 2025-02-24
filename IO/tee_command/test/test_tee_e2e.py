from subprocess import run 


def test_dummy(binary): 
    run([binary, '--help'])
