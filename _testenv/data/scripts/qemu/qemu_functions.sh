#!/bin/sh -ex
INITRD_DIR="$PWD/_initrd"
CORE_DIR="$PWD/_coredump"

# Add one or more files to the initramfs, with parent directories.
# usr-merge: resolve symlinks for /lib -> /usr/lib etc. so "cp --parents" does
# not fail with "cp: cannot make directory '/tmp/initrd/lib': File exists"
# $@: path to files
qemu_initrd_add_file() {
	local i

	for i in "$@"; do
		case "$i" in
		/bin/*|/sbin/*|/lib/*|/lib64/*)
			cp -a --parents "$i" "$INITRD_DIR"/usr
			;;
		*)
			cp -a --parents "$i" "$INITRD_DIR"
			;;
		esac
	done
}

# Add kernel module files with dependencies
# $@: kernel module names
qemu_initrd_add_mod() {
	if [ "$TESTENV_QEMU_KERNEL" != "debian" ]; then
		# Assume all drivers are statically built into the kernel, when
		# using a custom kernel
		return
	fi

	local kernel="$(basename /lib/modules/*)"
	local files="$(modprobe \
		-a \
		--dry-run \
		--show-depends \
		--set-version="$kernel" \
		"$@" \
		| sort -u \
		| cut -d ' ' -f 2)"

	qemu_initrd_add_file $files

	# Save the list of modules
	for i in $@; do
		echo "$i" >> "$INITRD_DIR"/modules
	done
}

# Add binaries with depending libraries
# $@: paths to binaries
qemu_initrd_add_bin() {
	local bin
	local bin_path
	local file

	for bin in "$@"; do
		local bin_path="$(which "$bin")"
		if [ -z "$bin_path" ]; then
			echo "ERROR: file not found: $bin"
			exit 1
		fi

		lddtree_out="$(lddtree -l "$bin_path")"
		if [ -z "$lddtree_out" ]; then
			echo "ERROR: lddtree failed on '$bin_path'"
			exit 1
		fi

		for file in $lddtree_out; do
			qemu_initrd_add_file "$file"

			# Copy resolved symlink
			if [ -L "$file" ]; then
				qemu_initrd_add_file "$(realpath "$file")"
			fi
		done
	done
}

# Add command to run inside the initramfs
# $@: commands
qemu_initrd_add_cmd() {
	local i

	if ! [ -e "$INITRD_DIR"/cmd.sh ]; then
		echo "#!/bin/sh -ex" > "$INITRD_DIR"/cmd.sh
		chmod +x "$INITRD_DIR"/cmd.sh
	fi

	for i in "$@"; do
		echo "$i" >> "$INITRD_DIR"/cmd.sh
	done
}

qemu_initrd_init() {
	mkdir "$INITRD_DIR"

	for dir in bin sbin lib lib64; do
		ln -s usr/"$dir" "$INITRD_DIR"/"$dir"
	done

	mkdir -p \
		"$INITRD_DIR"/dev/net \
		"$INITRD_DIR"/proc \
		"$INITRD_DIR"/sys \
		"$INITRD_DIR"/tmp \
		"$INITRD_DIR"/usr/bin \
		"$INITRD_DIR"/usr/sbin

	qemu_initrd_add_bin \
		busybox

	qemu_initrd_add_cmd \
		"export PATH='$PATH:\$PATH'"

	if [ "$TESTENV_QEMU_KERNEL" = "debian" ]; then
		qemu_initrd_add_mod \
			virtio_net \
			virtio_pci

		qemu_initrd_add_file \
			/lib/modules/*/modules.dep
	fi
}

qemu_initrd_finish() {
	cp "$TESTENV_QEMU_SCRIPTS"/qemu_init.sh "$INITRD_DIR"/init

	tree -a "$INITRD_DIR"

	( cd "$INITRD_DIR"; find . -print0 \
		| cpio --quiet -o -0 -H newc \
		| gzip -1 > "$INITRD_DIR".gz )
}

qemu_initrd_exit_error() {
	# Building the initrd is quite verbose, therefore put it in a log file
	# and only output its contents on error (see e.g. osmo-ggsn/run.sh)
	cat "$1"
	set +x
	echo
	echo "ERROR: failed to build the initrd!"
	echo
	exit 1
}

qemu_random_mac() {
	printf "52:54:"
	date "+%c %N" | sha1sum | sed 's/\(.\{2\}\)/\1:/g' | cut -d: -f 1-4
}

qemu_run() {
	local machine_arg
	local kernel="$TESTENV_QEMU_KERNEL"
	local kernel_cmdline="
		root=/dev/ram0
		console=ttyS0
		panic=-1
		init=/init
		loglevel=8
	"

	if [ "$kernel" = "debian" ]; then
		kernel="$(ls -1 /boot/vmlinuz*)"
	fi

	if [ -e /dev/kvm ]; then
		machine_arg="-machine pc,accel=kvm"
	else
		machine_arg="-machine pc"
	fi

	mkdir -p "$CORE_DIR"

	# sudo is required to set up networking in qemu_ifup.sh
	# </dev/null to deatch stdin, so qemu doesn't capture ^C
	sudo sh -c "
		qemu-system-x86_64 \
			$machine_arg \
			-smp 1 \
			-m 512M \
			-no-user-config -nodefaults -display none \
			-no-reboot \
			-kernel '$kernel' \
			-initrd '$INITRD_DIR.gz' \
			-append '$kernel_cmdline' \
			-serial stdio \
			-netdev 'tap,id=nettest,script=$TESTENV_QEMU_SCRIPTS/qemu_ifup.sh' \
			-device 'virtio-net-pci,netdev=nettest,mac=$(qemu_random_mac)' \
			-virtfs 'local,path=$CORE_DIR,mount_tag=coredir,security_model=passthrough,fmode=666,writeout=immediate' \
		</dev/null
	"

	# Show coredump backtrace
	if [ -e "$CORE_DIR/coredump" ]; then
		execfn="$(file "$CORE_DIR/coredump" | grep -o "execfn: '.*'" | cut -d "'" -f 2)"
		gdb --batch "$execfn" "$CORE_DIR/coredump" -ex bt
	fi
}
