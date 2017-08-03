#!/bin/bash

until latte-dock; do
echo "'latte-dock' crashed with exit code $?. Respawning.." >&2
sleep 2
done
