if [ -n "${GITHUB_TOKEN}" ]; then
    cd "$TRAVIS_BUILD_DIR"
    mkdir doc/sphinx/build
    docker run -v $TRAVIS_BUILD_DIR/doc/sphinx:/sphinx ldumas/sirius_dockerfile:sirius_pages /bin/sh -c "cd /sphinx; make html"
    echo "ls -l de sphinx pour voir les droits sur build"
    ls -l $TRAVIS_BUILD_DIR/doc/sphinx/
    cd $TRAVIS_BUILD_DIR/doc/sphinx/build/
    echo $PWD
    ls -l
    touch .nojekyll
    git init
    git checkout -b gh-pages
    git add .
    git -c user.name='travis' -c user.email='travis' commit -m init
    # Make sure to make the output quiet, or else the API token will leak!
    # This works because the API key can replace your password.
    git push -q -f https://dumasl:${GITHUB_TOKEN}@github.com/dumasl/SIRIUS gh-pages &2>/dev/null
    #git push -f https://dumasl:${GITHUB_TOKEN}@github.com/dumasl/SIRIUS gh-pages
    cd "$TRAVIS_BUILD_DIR"
fi

