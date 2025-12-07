from setuptools import setup, Extension
import sysconfig

def main():
    CFLAGS = ['-g', '-Wall', '-std=c99', '-fopenmp', '-mavx', '-mfma', '-pthread', '-O3']
    LDFLAGS = ['-fopenmp']
    # Use the setup function we imported and set up the modules.
    # You may find this reference helpful: https://docs.python.org/3.6/extending/building.html
    # copy from gemini im sorry
    module = Extension(
        'numc',
        sources=['numc.c', 'matrix.c'],
        extra_compile_args=CFLAGS,
        extra_link_args=LDFLAGS
    )
    setup(
        name='numc',
        version='1.0',
        description='A matrix library for CS61C, built as a Python C extension.',
        ext_modules=[module]
    )

if __name__ == "__main__":
    main()
