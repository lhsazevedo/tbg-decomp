# DSL Reference

Full reference for the `sh4objtest` `TestCase` DSL. See [`AGENTS.md`](AGENTS.md) for the workflow and guardrails.

## Symbol Management

```php
$this->setSize('_symbol', 0x6c);            // Reserve space for a symbol
$this->addressOf('_symbol');                 // Get allocated address
$this->alloc(0x34);                          // Allocate N bytes, returns address
$this->allocString("hello");                 // Allocate null-terminated string
$this->rellocate('_symbol', $addr);          // Place symbol at specific address
```

`setSize` works for external functions too (size 4); skipping this triggers an unresolved relocation.

## Seeding Memory

```php
$this->initUint8($addr, $value);
$this->initUint16($addr, $value);
$this->initUint32($addr, $value);
$this->doNotRandomizeMemory();               // Disable random fill (use sparingly)
```

## Invoking the Entrypoint

```php
$this->call('_FuncName_8c012718');
$this->call('_FuncName_8c012718')->with($arg1, $arg2);
$this->call('_FuncName_8c012718')->with(new WildcardArgument(), $arg2);
```

## Asserting Calls

```php
$this->shouldCall('_fn');
$this->shouldCall('_fn')->with($a, $b);
$this->shouldCall('_fn')->andReturn($value);
$this->shouldCall('_fn')->with($a)->andReturn($value);

// Callback for complex side effects (e.g. memcpy simulation)
$this->shouldCall('__quick_evn_mvn')->do(function ($params) {
    $src = $this->registers[2]->value;
    $dst = $this->registers[1]->value;
    $len = $this->registers[0]->value;
    for ($i = 0; $i < $len; $i++) {
        $this->memory->writeUInt8($dst + $i, $this->readUInt8($src + $i));
    }
});
```

## Asserting Memory Effects

```php
// Writes
$this->shouldWriteByte($addr, $value);
$this->shouldWriteWord($addr, $value);
$this->shouldWriteLong($addr, $value);
$this->shouldWriteFloat($addr, $floatValue);    // Encodes as IEEE 754
$this->shouldWriteString($addr, "text");

// Writes to named symbol (shorthand for addressOf + write)
$this->shouldWriteLongTo('_var_name_8c1ba358', $value);

// Writes at symbol + offset
$this->shouldWriteLongSymbolOffset('_symbol', $offset, $value);

// Reads
$this->shouldReadByte($addr);
$this->shouldReadWord($addr);
$this->shouldReadLong($addr);
$this->shouldReadLongFrom('_var_name_8c225fb8', $value);
```

## Control

```php
$this->shouldReturn($value);    // Assert return value (R0)
$this->forceStop();             // Stop execution when all prior expectations are met
```

## Patterns

### Struct pointer parameter
```php
$task = $this->alloc(0x20);
$this->initUint32($task + 0x08, 0);
$this->call('_MyTask_8c017718')->with($task);
```

### Loop assertions
```php
$base = $this->addressOf('_init_courseMenuButtons_8c04442c');
$indices = [2, 3, 4, 7, 8, 9];
foreach ($indices as $i => $idx) {
    $this->shouldWriteLong($base + $idx * 0x1c + 0x04, $baseVal + $i);
}
```

### Float helper
```php
// Convert float to raw uint32 for initUint32 if needed
$raw = unpack('L', pack('f', 42.5))[1];
```

### Helper methods (extract when repeated 3+ times)
```php
private function shouldDrawSprite(int $no, float $x, float $y, float $z): void {
    $this->shouldCall('_drawSprite_8c014f54')->with(
        $this->addressOf('_menuState_8c1bc7a8') + 0x0c, $no, $x, $y, $z
    );
}

private function shouldWriteMenuState(int $state): void {
    $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, $state);
}
```

### Manual symbol relocation
```php
$this->rellocate('_var_tasks_8c1ba5e8', $tasksPtr);
```
