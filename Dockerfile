FROM archlinux:latest
ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=/opt/devkitpro/devkitARM
ENV DEVKITPPC=/opt/devkitpro/devkitPPC
RUN pacman-key --init
RUN pacman -Syu --noconfirm
RUN pacman-key --recv BC26F752D25B92CE272E0F44F7FD5492264BB9D0 --keyserver keyserver.ubuntu.com
RUN pacman-key --lsign BC26F752D25B92CE272E0F44F7FD5492264BB9D0
RUN pacman -U https://pkg.devkitpro.org/devkitpro-keyring.pkg.tar.zst --noconfirm
RUN pacman-key --populate devkitpro
RUN echo "[dkp-libs]" >> /etc/pacman.conf
RUN echo "Server = https://pkg.devkitpro.org/packages" >> /etc/pacman.conf
RUN echo "[dkp-linux]" >> /etc/pacman.conf
RUN echo "Server = https://pkg.devkitpro.org/packages/linux/x86_64/" >> /etc/pacman.conf
RUN pacman -Syu base-devel gba-dev --noconfirm
WORKDIR /app
# COPY . .
CMD make
