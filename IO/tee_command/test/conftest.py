import pytest 


def pytest_addoption(parser):
    parser.addoption("--binary", required=True)


@pytest.fixture(scope="session")
def binary(pytestconfig):
    return pytestconfig.getoption("binary")