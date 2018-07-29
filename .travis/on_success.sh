if [ -n "${GITHUB_TOKEN}" ]; then
    cd "$TRAVIS_BUILD_DIR"
    mkdir doc/sphinx/build
    docker run -v $TRAVIS_BUILD_DIR/doc/sphinx:/sphinx ldumas/sirius_dockerfile:sirius_pages /bin/sh -c "cd /sphinx; make html"
    cd $TRAVIS_BUILD_DIR/doc/sphinx/build/
    touch .nojekyll
    git init
    git checkout -b gh-pages
    git add .
    git -c user.name='travis' -c user.email='travis' commit -m init
    git push -q -f https://dumasl:${GITHUB_TOKEN}@github.com/dumasl/SIRIUS gh-pages &2>/dev/null
    #git push -f https://dumasl:${GITHUB_TOKEN}@github.com/dumasl/SIRIUS gh-pages
    cd "$TRAVIS_BUILD_DIR"
fi

