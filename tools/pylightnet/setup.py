import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="pylightnet-favoryoung",
    version="0.1.0",
    author="Zhixu Zhao",
    author_email="zhixu001@126.com",
    description="A Python wrapper for Lightnet",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/zhaozhixu/lightnet",
    packages=setuptools.find_packages(),
    classifiers=[
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
    ],
)
