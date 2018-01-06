from setuptools import setup


setup(name='aesattack',
      version='0.0.2',
      description=u"analyze AES key by statistic data",
      classifiers=[],
      keywords='',
      author=u"Bo Li",
      author_email='leborn@yeah.net',
      license='zlib',
      packages=['tworoundattack'],
      zip_safe=False,
      install_requires=[
          'click',
          'matplotlib',
          'scipy',
          'numpy',
          'gf256',
      ],
      entry_points="""
      [console_scripts]
      aesattack=tworoundattack.aesattack:main
      """
)
