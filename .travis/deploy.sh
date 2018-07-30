#!/bin/sh
cat >Dockerfile <<EOF
FROM ldumas/sirius_dockerfile:sirius_built

ENTRYPOINT ["/opt/sirius/bin/sirius"]

EOF

# build
docker build -t sirius .
# tag the image
docker tag sirius ldumas/sirius_dockerfile:sirius
# push all ldumas/sirius_dockerfile images
docker push ldumas/sirius_dockerfile:sirius



